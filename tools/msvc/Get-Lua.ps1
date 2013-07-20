Param(
    [Parameter(Mandatory=$True)]
    [string]$version
)

$myPath = (split-path -parent $MyInvocation.MyCommand.Definition) + "`\"

$filename = "lua-" + $version + ".tar.gz"
$url = "http://www.lua.org/ftp/" + $filename

$wc = New-Object System.Net.WebClient
$wc.DownloadFile($url, $myPath + $filename)

C:\Program` Files\7-Zip\7z.exe e ($myPath + $filename)
$tar = $myPath + "lua-" + $version + ".tar"
C:\Program` Files\7-Zip\7z.exe -r -o"..\\.." x $tar
