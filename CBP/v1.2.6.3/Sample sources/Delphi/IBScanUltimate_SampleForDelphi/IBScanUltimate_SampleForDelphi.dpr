program IBScanUltimate_SampleForDelphi;

uses
  Forms,
  SDKMainForm in 'SDKMainForm.pas' {Form1},
  IBScanUltimate in 'IBScanUltimate.pas',
  IBScanUltimateApi_err in 'IBScanUltimateApi_err.pas',
  IBScanUltimateApi_defs in 'IBScanUltimateApi_defs.pas',
  IBScanUltimateApi in 'IBScanUltimateApi.pas';

{$R *.res}

begin
  Application.Initialize;
  Application.CreateForm(TForm1, Form1);
  Application.Run;
end.
