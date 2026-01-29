using Newtonsoft.Json;
using System.IO;

namespace CvBuilder.Ui.Util;

public static class JsonHelpers
{
    /// <summary>
    /// Writes an object to a file as a JSON string. Overrites the file if it already exists.
    /// </summary>
    /// <param name="obj"></param>
    /// <param name="path"></param>
    public static void WriteToPathAsJson(this object obj, string path)
    {
        var json = obj.SerializeAsJson();
        if (File.Exists(path)) File.Delete(path);
        File.WriteAllText(path, json);
    }

    public static string? SerializeAsJson(this object obj)
    {
        var settings = new JsonSerializerSettings
        {
            TypeNameHandling = TypeNameHandling.Auto,
            Formatting = Formatting.Indented
        };
        var json = JsonConvert.SerializeObject(obj, settings);
        return json;
    }

    public static T? DeserializeFromJson<T>(this string json)
    {
        var settings = new JsonSerializerSettings
        {
            TypeNameHandling = TypeNameHandling.Auto,
            Formatting = Formatting.Indented
        };
        var obj = JsonConvert.DeserializeObject<T>(json, settings);
        return obj;
    }
}