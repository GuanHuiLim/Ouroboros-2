# Define the root directory to start searching
$rootDirectory = $PSScriptRoot 
Write-Host 
Write-Host "$($rootDirectory)"

Get-Location
Push-Location $($rootDirectory)
Set-Location $rootDirectory

$s = $Env:vulkan_sdk

$compilerPath = "$($Env:vulkan_sdk)\Bin\glslc.exe"
Write-Host "Compiler path is : $($compilerPath)"

# Get a list of all GLSL shader files in the current directory
$shaderFiles = Get-ChildItem -Path . -Filter *.vert

# Define the list of files
$files = @(
    "debugdraw.vert",
	"deferredBoxLighting.vert",
	"deferreddecal.vert",
	"deferredlighting.vert",
	"forwarddecal.vert",
	"forwardParticles.vert",
	"forwardUI.vert",
	"gbuffer.vert",
	"sky.frag",
	"farplaneFullscreen.vert",
	"genericFullscreen.vert",
	"shadow.vert",
	"blit.frag",
	"debugdraw.frag",
	"deferredBoxLighting.frag",
	"deferreddecal.frag",
	"deferredlighting.frag",
	"forwarddecal.frag",
	"forwardParticles.frag",
	"forwardUI.frag",
	"gbuffer.frag",
	"shadow.frag",
	"zprepass.vert",
	"ssao.frag",
	"ssaoBlur.frag",
	"additiveComposite.comp",
	"irradiance.comp",
	"envPrefilter.comp",
	"brdfLUT.comp",
	"cdfscan.comp",
	"histogram.comp",
	"brightPixels.comp",
	"computeCull.comp",
	"downsample.comp",
	"fxaa.comp",
	"shadowPrepass.comp",
	"tonemapping.comp",
	"upsample.comp",
	"vignette.comp"
    # Add more file paths as needed
)


Write-Host "StartingCompile"
# Process files in parallel using ForEach-Object
$files | ForEach-Object -Parallel{
    $file = $_

    # Your script block here to process each file in parallel
	$absolutePath = Resolve-Path -Path $file
    $content = Get-Content -Path $absolutePath
    Write-Host "Processing file $file on thread $([System.Threading.Thread]::CurrentThread.ManagedThreadId)"
	
    $compilerPath = "$($Env:vulkan_sdk)\Bin\glslc.exe"
	$compilerArgs = @("--target-env=vulkan1.3", "-std=460", "-O", "$($file)", "-o", "bin/$($file).spv")
	
	& $compilerPath $compilerArgs
    # Add your processing logic here
} -ThrottleLimit 12  # Adjust the throttle limit as needed


# Define the list of files
$fidelityFiles = @(
    "spd\ffx_spd_downsample_pass.glsl"	
    # Add more file paths as needed
)

$fidelityFiles | ForEach-Object -Parallel{
    $file = $_

	$inputFile = "fidelity\src\backends\vk\shaders\$($file)"
	$absolutePath = Resolve-Path -Path $inputFile
    $content = Get-Content -Path $absolutePath
    Write-Host "Processing file $file on thread $([System.Threading.Thread]::CurrentThread.ManagedThreadId)"
	
	$fileName = Split-Path $file -Leaf
	Write-Host $inputFile
	Write-Host $fileName
	
    $compilerPath = "$($Env:vulkan_sdk)\Bin\glslc.exe"
	$compilerArgs = @("--target-env=vulkan1.3","-fshader-stage=comp", "-std=450", "-O", "$($inputFile)", "-o", "bin/$($fileName).spv","-DFFX_GLSL","-DFFX_GPU","-I", "fidelity\include\FidelityFX\gpu")
	
	& $compilerPath $compilerArgs
    # Add your processing logic here
} -ThrottleLimit 12  # Adjust the throttle limit as needed

Write-Host "EndCompile"
Pop-Location
exit

