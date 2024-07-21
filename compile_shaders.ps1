$path = ".\assets\shaders\"
$bin_path = ".\bin\Debug\"
$fragments = Get-ChildItem -Path "$($path)*.frag"
$vertices = Get-ChildItem -Path "$($path)*.vert"

foreach ($f in $vertices) {
	if ($f.Attributes -band [Io.FileAttributes]::Archive) {
		$output_name = "$($bin_path)$($f.Name)".Replace(".vert", ".spv")
		$shader_name = "$($path)$($f.Name)"
		Start-Process -FilePath "$($env:vulkan_sdk)\Bin\glslc.exe" -ArgumentList "-c $($shader_name) -o $($output_name)"
	}
}

foreach ($f in $fragments) {
	if ($f.Attributes -band [Io.FileAttributes]::Archive) {
		$output_name = "$($bin_path)$($f.Name)".Replace(".frag", ".spv")
		$shader_name = "$($path)$($f.Name)"
		Start-Process -FilePath "$($env:vulkan_sdk)\Bin\glslc.exe" -ArgumentList "-c $($shader_name) -o $($output_name)"
	}
}

