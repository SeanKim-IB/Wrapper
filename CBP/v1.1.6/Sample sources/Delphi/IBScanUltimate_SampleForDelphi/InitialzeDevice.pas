unit InitialzeDevice;

interface

uses
  Classes;

type
  InitializeDeviceThread = class(TThread)
  private
    { Private declarations }
  protected
    procedure Execute; override;
  public
    constructor Create;
  end;

var
  InitialThread: InitializeDeviceThread;
implementation

{ Important: Methods and properties of objects in visual components can only be
  used in a method called using Synchronize, for example,

      Synchronize(UpdateCaption);

  and UpdateCaption could look like,

    procedure InitializeDeviceThread.UpdateCaption;
    begin
      Form1.Caption := 'Updated in a thread';
    end; }

{ InitializeDeviceThread }

procedure InitializeDeviceThread.Execute;
begin
  while not Terminated do
  begin
//    Form1.Caption := 'aa';
  end;
end;

constructor InitializeDeviceThread.Create;
begin
  inherited Create(false);
  Priority := tpTimeCritical;
end;

end.


