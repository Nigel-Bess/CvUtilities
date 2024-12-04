#!/bin/bash

function display_help() {
    echo "Usage: $0 -n <namespace> -d <deployment_name> <remote_path> <local_path>"
    echo
    echo "Options:"
    echo "  -n    Kubernetes namespace where the deployment is located (DEFAULT: fulfil-prod)"
    echo "  -d    Name of the deployment."
    echo "  -h    Display this help message."
    echo
    echo "Positional arguments:"
    echo "  <remote_path>   Remote path on the pod from which files will be copied."
    echo "  <local_path>    Local path where files will be copied to."
    echo
    echo "Example:"
    echo "  $0 -n default -d my-deployment /path/on/pod /path/on/local"
    exit 1
}

namespace="fulfil-prod"

while getopts "n:d:h" opt; do
    case ${opt} in
        n ) namespace=$OPTARG ;;
        d ) deployment_name=$OPTARG ;;
        h ) display_help ;;
        * ) display_help ;;
    esac
done

shift $((OPTIND -1))

if [[ -z "$1" || -z "$2" ]]; then
    echo "Error: Missing required remote and local directory arguments."
    display_help
fi

remote_path=$1
local_path=$2

if [[ -z "$namespace" || -z "$deployment_name" ]]; then
    echo "Error: Missing required arguments for namespace or deployment."
    display_help
fi

pod_name=$(kubectl get pods -n $namespace -l app.kubernetes.io/name=$deployment_name -o jsonpath="{.items[0].metadata.name}")

if [[ -z "$pod_name" ]]; then
    echo "Error: Unable to find a pod for the deployment $deployment_name in namespace $namespace."
    kubectl config get-contexts
    echo "Are you using the right Kubernetes context?"
    echo '  Change your context with `kubectl config use-context CONTEXT_NAME`'
    exit 1
fi

kubectl cp -c cv-repack "$namespace/$pod_name:$remote_path" "$local_path"

if [ $? -ne 0 ]; then
    echo "Error copying files."
    exit 1
fi
