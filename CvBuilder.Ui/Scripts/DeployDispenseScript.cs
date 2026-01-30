using CvBuilder.Ui.DeployDispense;
using CvBuilder.Ui.Hardcoded;
using CvBuilder.Ui.Terminal;
using System.IO;

namespace CvBuilder.Ui.Scripts;

internal class DeployDispenseScript : CombinedScript
{
    public override string Name { get; }

    private readonly DeployableBuild _build;
    private readonly Dispense _dispense;
    public DeployDispenseScript(DeployableBuild build, Dispense dispense)
    {
        Name = $"Deploy {build} to {dispense}";
        _build = build;
        _dispense = dispense;
    }

    public override IEnumerable<IScript> SubSteps()
    {
        var sshAuth = _dispense.Login;
        yield return new EditRemoteFileScript("Edit docker compose", sshAuth, "~/code/Fulfil.ComputerVision/docker-compose.dab.yml", EditFile, keepSshAlive: true);
        yield return BasicTextCommand.MultiLine([
            "cd ~/code/Fulfil.ComputerVision/",
            "docker compose -f docker-compose.dab.yml pull",
            "docker compose -f docker-compose.dab.yml down",
            "docker compose -f docker-compose.dab.yml up -d --remove-orphans",
            ]);
        yield return new GenericScript("Wait for deploy", WaitForDeployToFinish);
        yield return new BasicTextCommand("exit");
    }

    private string EditFile(string remoteFileContents)
    {
        static YamlDotNet.RepresentationModel.YamlScalarNode S(string v) => new(v);

        var yaml = new YamlDotNet.RepresentationModel.YamlStream();
        yaml.Load(new StringReader(remoteFileContents));

        var root = (YamlDotNet.RepresentationModel.YamlMappingNode)yaml.Documents[0].RootNode;
        var services = (YamlDotNet.RepresentationModel.YamlMappingNode)root.Children[S("services")];
        var depthcam = (YamlDotNet.RepresentationModel.YamlMappingNode)services.Children[S("depthcam")];

        depthcam.Children[S("image")] = S($"gcr.io/fulfil-web/cv-dispense/{_build.ImageIdentifier()}");

        using var sw = new StringWriter();
        yaml.Save(sw, assignAnchors: false);
        return sw.ToString();
    }

    private async Task<ScriptCompletionInfo> WaitForDeployToFinish(TerminalViewModel terminal)
    {
        var maxTimeMs = (int)TimeSpan.FromMinutes(1).TotalMilliseconds;
        var deploySuccess = terminal.AwaitText("Running 10/10", maxTimeMs);
        if (!await deploySuccess) return ScriptCompletionInfo.Failure("Deploy failed!");
        return ScriptCompletionInfo.Success;
    }
}
