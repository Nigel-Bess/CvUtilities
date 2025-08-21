import random
from _3_generate_fed_cvat import generate_fed_cvat_from_test
from _4_fed_summarize import eval_output, score_results
from vizier import service
from vizier.service import clients
from vizier.service import pyvizier as vz
from vizier.service import servers
import subprocess
import shutil
import os
import time
from multiprocessing.pool import ThreadPool

WORKERS = 1

# Step 1: Pick from some (sub)set of these FED params to optimize:
hyperparams = [
    #["FED_LOWEST_GROUND_Z", (0.0, 0.8), "float"], # Calibrated Z may be up too high by this much
    #["FED_HIGHEST_CEILING_Z", (-1, 0.1), "float"], # Calibrated Z may be too low by this much
    ["FED_MAX_ITEM_HEIGHT_RATIO", (0.5, 0.99), "float"], # Ceiling of true ground may be up to this fraction of item height
    ["FED_HIGHEST_PROPOSAL_BIAS", (0.49, 0.99), "float"], # Between true ground bounds, bias toward ceil proposal this much
    ["FED_LOWEST_PROPOSAL_BIAS", (0.01, 0.49), "float"], # Between true ground bounds, bias toward ceil proposal this much
    ["FED_LOW_PROPOSAL_SCALAR", (0.01, 0.99), "float"], # Between true ground bounds, bias toward floor proposal this much
    ["FED_MIN_ITEM_Z_THICKNESS", (0.00001, 0.001), "float"], # Min height of an item the depthcam can differentiate from ground points
    #["FED_MAX_LINE_Y_DIFF", (0.01, 0.09), "float"], # Max diff between predicted Ys for 2 lane lines to agree
    #["FED_EMPTY_CONSENSUS_COUNT", (12, 18), "int"], # Number of lines that must claim empty lane before believing it
    #["FED_CONSENSUS_COUNT", (1, 7), "float"], # Number of lines that must roughly agree on front edge Y point
    #["FED_LINE_SPREAD_MULTIPLIER", (-3, 3), "float"] # Spread of X across lane line evaluations
]
# CHANGE THIS but also you get unique params leading to unique trials for free
TRIAL_NAME = 'fa2july'
bounds = [p[1] for p in hyperparams]

best_score = -99999999
study = None

def get_dataset_copy_dir(dataset, i):
    return "data/" + dataset + "-" + TRIAL_NAME + "-" + str(i)

# Run FED against all test samples
def eval_all(params, worker_id):
    global best_score
    dataset = os.environ['DATASET_NAME']

    inst = dataset + "-test-" + TRIAL_NAME + "-" + str(worker_id)
    dataset_dir = get_dataset_copy_dir(dataset, worker_id)
    env_str = ' -e TEST_INSTANCE={} -e TEST_SRC="{}"'.format(inst, dataset_dir)
    debug = ""
    for p in range(0, len(hyperparams)):
        param_name = hyperparams[p][0]
        param_val = params[p]
        env_str += " -e " + param_name + "=" + format(param_val, 'f')
        debug += param_name + " = " + format(param_val, 'f') + "\n"

    run_cmd = 'docker --log-level ERROR compose run ' + env_str + ' --quiet --remove-orphans dispense_eval "Fulfil.Dispense/app/eval" fed all > /dev/null'
    # Due to file race bugs, keep re-running till it works without errors
    exit_code = 1
    start_time = time.time()
    while exit_code != 0:
        start_time = time.time()
        call = subprocess.run(run_cmd, capture_output=True, shell=True)
        exit_code = call.returncode
        if exit_code != 0:
            print("Ran test" + str(worker_id) + " and got exit code " + str(exit_code) + ", retrying...")
    
    end_time = time.time()
    # Generate CVAT annotations from test output
    generate_fed_cvat_from_test(test_instance=inst, verbose=False)
    count, correct_count, correct_y_highs, correct_y_lows, \
        false_pos, false_neg, overdispenses, underdispenses, out_vs_true_points = eval_output("data/cvat/FED/" + inst + "/annotations.xml", "metrics/" + dataset + "/FED-ground-truth.xml")
    
    score = score_results(count, correct_y_highs, correct_y_lows, overdispenses, underdispenses, false_neg)
    print("===== Score: " + '{0:.4f}'.format(score) + " vs " + '{0:.4f}'.format(best_score) + " in " + str(int(end_time - start_time)) + "s" + " id:" + str(worker_id))

    if score > best_score:
        best_score = score
        print("\n\n =========== Best score " + str(score) + "! =============\n")
        print("Env:\n" + debug)

    return score

def tuning_loop(study, worker_id):
    global hyperparams
    # Run the tuning loop
    while True:
        suggestions = study.suggest(count=1)
        for suggestion in suggestions:
            params = suggestion.parameters
            val_list = []
            for i in range(0, len(hyperparams)):
                val_list.append(params[hyperparams[i][0]])
            objective = eval_all(val_list, worker_id)
            suggestion.complete(vz.Measurement({'score': objective}))

def main():
    dataset = os.environ['DATASET_NAME']
    avg_eval_all_runtime_secs = 60
    max_runtime_secs = 60 * 60 * 8 # Run for up to X hours
    max_evals = int(max_runtime_secs / avg_eval_all_runtime_secs)

    print("Hypertuning " + str(len(hyperparams)) + " FED params (TRIAL=" + TRIAL_NAME + ") against " + dataset + " with up to " + str(max_evals) + " evals or " + str(max_runtime_secs/60) + " minutes, proceed?")
    input()
    print("Setting up...")

    # Must copy entire input tree due to multithreading issues in OpenCV
    for i in range(0, WORKERS):
        dest = get_dataset_copy_dir(dataset, i)
        if not os.path.isdir(dest):
            print("Creating " + dest + " for parallel eval runs")
            shutil.copytree("data/" + dataset, dest)

    # Setup abstract Vizier OSS representation of hypertune problem
    study_config = vz.StudyConfig(algorithm='DEFAULT')
    for hparam in hyperparams:
        if hparam[2] == "float":
            study_config.search_space.root.add_float_param(hparam[0], hparam[1][0], hparam[1][1])
        elif hparam[2] == "int":
            study_config.search_space.root.add_int_param(hparam[0], hparam[1][0], hparam[1][1])
        else:
            raise Error("unhandled datatype " + hparam[2])
    study_config.metric_information.append(vz.MetricInformation(name='score', goal=vz.ObjectiveMetricGoal.MAXIMIZE))
    server = servers.DefaultVizierServer()
    clients.environment_variables.server_endpoint = server.endpoint
    study = clients.Study.from_study_config(study_config, owner='me', study_id=TRIAL_NAME)

    print("Starting pool...")
    with ThreadPool(WORKERS) as pool:
        pending = pool.starmap(tuning_loop, [[study, int(str(i))] for i in range(WORKERS)])
        for p in pending:
            p.wait()

    print("\nDone! results: \n")

    # Cleanup temp dirs
    for i in range(0, WORKERS):
        shutil.rmtree(get_dataset_copy_dir(dataset, i))
        shutil.rmtree("data/" + dataset + "-" + TRIAL_NAME + "-" + str(i))

if __name__ == '__main__':
    main()