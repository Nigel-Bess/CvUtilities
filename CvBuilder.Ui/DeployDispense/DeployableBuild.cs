using CvBuilder.Ui.Hardcoded;

namespace CvBuilder.Ui.DeployDispense;

public record DeployableBuild(string BranchName, Facility Facility)
{
    public bool IsValid() => Facility != Facility.None;
    public override string ToString() => $"{BranchName}:{Facility.GetDescription()}";
}
