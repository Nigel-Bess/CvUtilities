using CvBuilder.Ui.Hardcoded;

namespace CvBuilder.Ui.DeployDispense;

public record DeployableBuild(string BranchName, Facility Facility)
{
    public bool IsValid() => Facility != Facility.None;

    public string ImageIdentifier() => $"{BranchName}:{Facility.GetDescription()}";
    public override string ToString() => ImageIdentifier();
}
