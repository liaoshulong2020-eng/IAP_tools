using System.Reflection;

namespace IAPManager
{
    internal static class BootLoader5800DResources
    {
        internal static byte[] GetBin(string target) =>
            Read(target.Equals("PFC", StringComparison.OrdinalIgnoreCase)
                ? "BootLoader5800D.PFC.bin"
                : "BootLoader5800D.LLC.bin");

        internal static byte[] GetHex(string target) =>
            Read(target.Equals("PFC", StringComparison.OrdinalIgnoreCase)
                ? "BootLoader5800D.PFC.hex"
                : "BootLoader5800D.LLC.hex");

        private static byte[] Read(string resourceName)
        {
            using Stream? stream = Assembly.GetExecutingAssembly().GetManifestResourceStream(resourceName);
            if (stream == null)
            {
                throw new InvalidOperationException($"找不到内嵌资源: {resourceName}");
            }

            using MemoryStream buffer = new MemoryStream();
            stream.CopyTo(buffer);
            return buffer.ToArray();
        }
    }
}
