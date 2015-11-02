"C:\Program Files (x86)\MSBuild\14.0\Bin\msbuild.exe" ..\AdcMcp3008.csproj /target:Clean;Build /p:Configuration=Release /p:Platform=AnyCPU

if exist lib (rmdir lib /s /q)
mkdir lib\uap10.0

copy ..\bin\Release\*.pri lib\uap10.0
copy ..\bin\Release\*.winmd lib\uap10.0

e:\nuget\nuget pack Microsoft.IoT.AdcMcp3008.nuspec
