; PromptEditor — Inno Setup installer script
;
; Generates a graphical Windows installer (pp-setup-<version>.exe).
;
; Prerequisites: Install Inno Setup (https://jrsoftware.org/isinfo.php)
;                Run .\scripts\release.ps1 first to produce dist\pp.exe
;
; Build from the repository root:
;   iscc scripts\installer.iss
;
; Or with a version override:
;   iscc /DAppVersion=0.2.0 scripts\installer.iss

#define AppName     "PromptEditor"
#define AppExeName  "pp.exe"
#define AppPublisher "PromptEditor Contributors"
#define AppURL       "https://github.com/wubing7755/PromptEditor"

; Version — read from CMakeLists.txt, or override with /DAppVersion=X.Y.Z
#ifndef AppVersion
#define AppVersion "0.1.0"
#endif

#ifndef SourceDir
#define SourceDir   "dist"
#endif

[Setup]
AppId={{B8F4A3D2-7E1C-4A5B-9F6D-1C2E3A4B5C6D}
AppName={#AppName}
AppVersion={#AppVersion}
AppPublisher={#AppPublisher}
AppPublisherURL={#AppURL}
AppSupportURL={#AppURL}/issues
AppUpdatesURL={#AppURL}/releases

DefaultDirName={autopf}\{#AppName}
DefaultGroupName={#AppName}
AllowNoIcons=yes

; Output file name
OutputDir=dist
OutputBaseFilename=pp-setup-{#AppVersion}

; Compression
Compression=lzma2/ultra64
SolidCompression=yes

; Visual style
WizardStyle=modern
WizardSizePercent=120,120

; Privileges — user can choose per-user or all-users
PrivilegesRequiredOverridesAllowed=dialog
UsePreviousPrivileges=yes
ArchitecturesInstallIn64BitMode=x64compatible

; Uninstaller
UninstallDisplayName={#AppName} {#AppVersion}
UninstallDisplayIcon={app}\{#AppExeName}

; License
LicenseFile=..\LICENSE

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
; PATH integration
Name: "addtopath"; Description: "&Add to PATH (recommended)"; GroupDescription: "PATH integration:"; Flags: checkedonce

; File association
Name: "associate"; Description: "&Associate .md files with pp edit (open .md files as prompts)"; GroupDescription: "File associations:"; Flags: unchecked

[Files]
; Main executable
Source: "{#SourceDir}\{#AppExeName}"; DestDir: "{app}"; Flags: ignoreversion

; Documentation
Source: "..\README.md"; DestDir: "{app}"; Flags: ignoreversion isreadme
Source: "..\LICENSE"; DestDir: "{app}"; Flags: ignoreversion

[Icons]
; Start Menu
Name: "{group}\PromptEditor"; Filename: "{app}\{#AppExeName}"; Parameters: "--help"
Name: "{group}\PromptEditor (interactive)"; Filename: "{app}\{#AppExeName}"; Parameters: "browse"
Name: "{group}\Uninstall PromptEditor"; Filename: "{uninstallexe}"

[Run]
; Launch interactive help on finish
Filename: "{app}\{#AppExeName}"; Parameters: "--version"; Description: "&Verify installation"; Flags: nowait postinstall skipifsilent runascurrentuser

[Registry]
; Add to user PATH (HKCU)
Root: HKCU; Subkey: "Environment"; ValueType: expandsz; ValueName: "Path"; \
    ValueData: "{olddata};{app}"; \
    Check: not IsAdminInstallMode() and WizardIsTaskSelected('addtopath'); \
    Tasks: addtopath

; Add to system PATH (HKLM)
Root: HKLM; Subkey: "SYSTEM\CurrentControlSet\Control\Session Manager\Environment"; \
    ValueType: expandsz; ValueName: "Path"; \
    ValueData: "{olddata};{app}"; \
    Check: IsAdminInstallMode() and WizardIsTaskSelected('addtopath'); \
    Tasks: addtopath

; Uninstall registry entry
Root: HKCU; Subkey: "Environment"; ValueType: expandsz; ValueName: "Path"; \
    ValueData: "{olddata};"; \
    Check: not IsAdminInstallMode() and WizardIsTaskSelected('addtopath'); \
    Flags: uninsremovesection

[Code]
// --- Custom path removal helper ---
function RemoveFromPath(const Path, Entry: string): string;
var
  Parts: TArrayOfString;
  i: Integer;
begin
  Result := '';
  Parts := SplitString(Path, ';');
  for i := 0 to GetArrayLength(Parts) - 1 do
  begin
    if Trim(Parts[i]) = Trim(Entry) then
      Continue;
    if Result <> '' then
      Result := Result + ';';
    Result := Result + Parts[i];
  end;
end;

// --- Uninstall: remove from PATH ---
procedure CurUninstallStepChanged(CurUninstallStep: TUninstallStep);
var
  CurrentPath: string;
  NewPath: string;
begin
  if CurUninstallStep = usPostUninstall then
  begin
    // Remove from user PATH
    if RegQueryStringValue(HKCU, 'Environment', 'Path', CurrentPath) then
    begin
      NewPath := RemoveFromPath(CurrentPath, ExpandConstant('{app}'));
      if NewPath <> CurrentPath then
        RegWriteStringValue(HKCU, 'Environment', 'Path', NewPath);
    end;
    // Remove from system PATH (if we had admin)
    if RegQueryStringValue(HKLM, 'SYSTEM\CurrentControlSet\Control\Session Manager\Environment', 'Path', CurrentPath) then
    begin
      NewPath := RemoveFromPath(CurrentPath, ExpandConstant('{app}'));
      if NewPath <> CurrentPath then
        RegWriteStringValue(HKLM, 'SYSTEM\CurrentControlSet\Control\Session Manager\Environment', 'Path', NewPath);
    end;
  end;
end;

// --- Welcome message ---
function InitializeWizard: Boolean;
begin
  Result := True;
end;
