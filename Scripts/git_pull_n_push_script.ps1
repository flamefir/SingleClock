# Prompt the user for input
Write-Host "This script will pull all, and push all changes."
$message = Read-Host "# Enter commit message"

while([string]::IsNullOrEmpty($message)){
    Write-Host "No commit message written."
    $message = Read-Host "# Enter commit message"
}
# Run the corresponding Git command
& git.exe status
& git.exe add --all
& git.exe commit -m $message  
& git.exe push -f
& git.exe status