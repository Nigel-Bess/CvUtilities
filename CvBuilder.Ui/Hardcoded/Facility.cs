using System.ComponentModel;

namespace CvBuilder.Ui.Hardcoded;

public enum Facility
{
    None = 0,
    [Description("pioneer")]
    Pioneer = 1,
    [Description("plm")]
    Plm = 2,
    [Description("whisman")]
    Whisman = 3,
    [Description("tan")]
    Tan = 4,
    [Description("plano")]
    Plano = 5,
}
