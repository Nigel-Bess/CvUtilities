using System.Collections.Immutable;

namespace CvBuilder.Ui.Hardcoded;

public record Dispense(string Name, string HostName, string Password, Facility Facility)
{
    private static ImmutableList<Dispense> Dispenses = ImmutableList.Create<Dispense>([
            new("P2","fulfil@p2-dab", "FreshEngr" ,Facility.Pioneer),
            new("P1","fulfil@p1-dab", "FreshEngr" ,Facility.Pioneer),
        ]);
    public static IEnumerable<Dispense> GetMachines(Facility facility) => Dispenses.Where(d => d.Facility == facility);
    public override string ToString()
    {
        return $"{Name} DAB";
    }
}
