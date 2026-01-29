using CvBuilder.Ui.Hardcoded;

namespace CvBuilder.Ui.Deploy;

public record DeployableBuild(string BranchName, Facility Facility)
{
    public bool IsValid() => Facility != Facility.None;
}
