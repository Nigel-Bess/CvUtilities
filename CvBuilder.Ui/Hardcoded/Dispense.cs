using System.Collections.Immutable;

namespace CvBuilder.Ui.Hardcoded;

public record Dispense(string Name, SshLogin Login, Facility Facility)
{
    private static ImmutableList<Dispense> Dispenses = ImmutableList.Create<Dispense>([
            new("P1",new("fulfil@p1-dab.pioneer.fulfil.ai","FreshEngr") ,Facility.Pioneer),
            new("P2",new("fulfil@p2-dab.pioneer.fulfil.ai","FreshEngr") ,Facility.Pioneer),
        ]);
    public static IEnumerable<Dispense> GetMachines(Facility facility) => Dispenses.Where(d => d.Facility == facility);
    public override string ToString()
    {
        return $"{Name} DAB";
    }
}
