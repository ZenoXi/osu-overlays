public class RequestData
{
    public string? BaseUrl { get; set; }
    public string? Path { get; set; }
    public Dictionary<string, string>? Headers { get; set; }

    public RequestData() { }

    public static void PreventTrimming()
    {
        var unused1 = new RequestData();
        unused1.BaseUrl = "-";
        unused1.Path = "-";
        unused1.Headers = new Dictionary<string, string>();
    }
}
