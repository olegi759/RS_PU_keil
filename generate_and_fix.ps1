# generate_and_fix.ps1 - генерация кода в CubeMX + автоматическое восстановление проекта + сборка.
# Запуск:  powershell -ExecutionPolicy Bypass -File .\generate_and_fix.ps1
$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent $MyInvocation.MyCommand.Path
$ioc  = Join-Path $root 'pu_new.ioc'
$mx   = 'D:\Programs\STM32CubeMX\STM32CubeMX.exe'
$before = (Get-Item (Join-Path $root '.mxproject')).LastWriteTime
$script = Join-Path $root '_gen_auto.txt'
Set-Content -Path $script -Value @(('config load "' + $ioc + '"'), 'project generate', 'exit') -Encoding ASCII
$out = Join-Path $root '_gen_auto_out.txt'
Start-Process -FilePath $mx -ArgumentList ('-q "' + $script + '"') -WorkingDirectory $root -RedirectStandardOutput $out -RedirectStandardError ($out + '.err') -NoNewWindow | Out-Null
Add-Type -TypeDefinition @"
using System;using System.Text;using System.Collections.Generic;using System.Runtime.InteropServices;
public class MxDlg {
  public delegate bool EnumProc(IntPtr h, IntPtr p);
  [DllImport("user32.dll")] public static extern bool EnumWindows(EnumProc cb, IntPtr p);
  [DllImport("user32.dll")] public static extern uint GetWindowThreadProcessId(IntPtr h, out uint pid);
  [DllImport("user32.dll")] public static extern int GetWindowText(IntPtr h, StringBuilder s, int n);
  [DllImport("user32.dll")] public static extern bool IsWindowVisible(IntPtr h);
  [DllImport("user32.dll")] public static extern bool PostMessage(IntPtr h, uint m, IntPtr w, IntPtr l);
  public static List<IntPtr> D(uint[] p){ var r=new List<IntPtr>(); EnumWindows((h,x)=>{ uint pid; GetWindowThreadProcessId(h,out pid); if(Array.IndexOf(p,pid)<0) return true; if(!IsWindowVisible(h)) return true; var t=new StringBuilder(512); GetWindowText(h,t,512); if(t.Length>0) r.Add(h); return true; }, IntPtr.Zero); return r; }
  public static void E(IntPtr h){ PostMessage(h,0x0100,(IntPtr)0x0D,(IntPtr)0x001C0001); System.Threading.Thread.Sleep(80); PostMessage(h,0x0101,(IntPtr)0x0D,(IntPtr)0xC01C0001); }
}
"@
Write-Host "=== Генерация кода в CubeMX ==="
$done = $false
for ($i=0; $i -lt 80; $i++) {
    Start-Sleep -Seconds 3
    $mxp = Get-Item (Join-Path $root '.mxproject') -ErrorAction SilentlyContinue
    if ($mxp -and $mxp.LastWriteTime -gt $before) { $done = $true; break }
    $pids = (Get-Process -Name javaw -ErrorAction SilentlyContinue | ForEach-Object { [uint32]$_.Id })
    if (-not $pids) { break }
    foreach ($h in [MxDlg]::D($pids)) { [MxDlg]::E($h); Write-Host "  подтвердил окно CubeMX (Enter)" }
}
if ($done) { Write-Host "  код сгенерирован" } else { Write-Host "  генерация не подтвердилась/не завершилась" -ForegroundColor Yellow }
Remove-Item $script -Force -ErrorAction SilentlyContinue
& powershell -ExecutionPolicy Bypass -File (Join-Path $root 'MDK-ARM\fix_after_cubemx.ps1')
