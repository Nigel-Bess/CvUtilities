using CvBuilder.Ui.DeployDispense;
using CvBuilder.Ui.Hardcoded;

namespace CvBuilder.Ui.Scripts;

internal class DeployDispenseScript : CombinedScript
{
    public override string Name { get; }

    private readonly DeployableBuild _build;
    private readonly Dispense _dispense;
    public DeployDispenseScript(DeployableBuild build, Dispense dispense)
    {
        Name = $"Deploy {build} to {dispense}";
    }

    public override IEnumerable<IScript> SubSteps()
    {
        yield return new Ssh(_dispense.Login);
        yield return BasicTextCommand.MultiLine([
            "cd ~/code/Fulfil.ComputerVision/",
            $"yq -i -y \".services.depthcam.image = \\\"{_build.ImageIdentifier()}\\\"\" docker-compose.dab.yml",
            "docker compose -f docker-compose.dab.yml pull",
            "docker compose -f docker-compose.dab.yml down",
            "docker compose -f docker-compose.dab.yml up -d --remove-orphans",
            ]);
    }
}
