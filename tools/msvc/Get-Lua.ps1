Param(
    [Parameter(Mandatory=$True)]
    [string]$version
)

$filename = "lua-" + $version + ".tar.gz"
$url = "http://www.lua.org/ftp/" + $filename

$wc = New-Object System.Net.WebClient
$wc.DownloadFile($url, $filename)

C:\Program` Files\7-Zip\7z.exe e $filename
$tar = "lua-" + $version + ".tar"
C:\Program` Files\7-Zip\7z.exe -r -o"..\\.." x $tar
