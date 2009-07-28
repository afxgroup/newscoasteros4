/******************************************************************************************

 A NewsCoaster bulk message decoder script
 Decodes all files in the currently active newsgroup.

 $VER: Bulk Decoder v1.0 (01.18.2004)

 Handling of multipart binaries was added by Roger Clark <kaedric@sbcglobal.net>

 This script should properly handle multipart yEnc and MIME Encapsulated Base64 files based
 on  data  contained  in  the  message  headers  and body.  If sections are out of order or
 missing, the script will report this as an error.

 Support for multipart UUEncoded files will detect the first part of the file properly, but
 assumes that the remaining parts are all in order already. The script will scan through as
 many attachments as it finds and decode each one.  When an attachment is split across more
 than one message, the script will continue appending to the last open temporary file until
 the next 'end' statement is reached.  If a section is missing, then the script has no real
 way of knowing this as UUEncoded files do not have any control data to indicate a multiple
 part message.

 Based on a script by "Jules" < create at clara co uk >

 This script requires the following libraries and commands.

 rexxtricks.library 38.6 by Jurgen Kohrmeyer
 rexxreqtools.library 37.95 by Rafael D'Halleweyn

 (put these commands in 'C:')
 amiydec by Ian Chapman           Aminet 'comm/mail/amiycoders.lha'
 maidecoder by James Ostrowick    Aminet 'dirs/aminet/util/cli/maildecoder.lha'

 usage: rx Bulk_Decoder.rexx [Directory]

 example: rx Bulk_Decoder.rexx Cache:Images/Downloaded/

 If you do not supply a desination directory a requester will be opened.

 *****************************************************************************************/

Options Results
Parse Arg Dest

If ~Show('P','NEWSCOASTER') Then Call ErrorMsg("NewsCoaster is not running!")

/****************************************************************************************/
/* This is the default directory for the output files, change as needed for your system */
/****************************************************************************************/

DefDest = "NewsCoasterData:"

/***********************************/
/* Load required support libraries */
/***********************************/

Lib.0 = 2
Lib.1.1 = 'rexxtricks.library'   ; Lib.1.2 = 38
Lib.2.1 = 'rexxreqtools.library' ; Lib.2.2 = 37

Do I = 1 to Lib.0
  If ~Show('L',Lib.I.1) Then If ~AddLib(Lib.I.1,0,-30,Lib.I.2) Then Do
    EM = Lib.I.1||" not found"
    Call ErrorMsg(EM)
  End
End

/****************************************************/
/* Find out where the decoded files should be saved */
/****************************************************/

If Dest = "" | ~Exists(Dest) Then Do
  Dest = RTFILEREQUEST(DefDest,,"Destination ?","OK","rtfi_flags=freqf_nofiles")
  If RTRESULT = 0 Then Call ErrorMsg("No directory selected")
End

Select
  When Pos(":",Dest) = 0 & Pos("/",Dest) = 0 Then Do
    BaseDir = Pragma('D')
    If Right(BaseDir,1) ~= ":" Then Dest = BaseDir||"/"Dest||"/"
    Else Dest = BaseDir||Dest||"/"
  End
  When Pos(Right(Dest,1),":/") = 0 Then Dest = Dest||"/"
  Otherwise NOP
End

Call Pragma("D",Dest)

/************************************************************************/
/* Find out if we should decode all messages, or only selected messages */
/************************************************************************/

DecodeMode = RTEZREQUEST('Decode which files in this newsgroup?','_All|_Marked|_Cancel')
If DecodeMode = 0 Then Exit

/****************************************************/
/* Find out what newsgroup we are currently reading */
/****************************************************/

Address 'NEWSCOASTER'

FOLDERINFO ; Parse VAR Result GID . NArticles .
Source = "NewsCoasterData:Folder_"||GID
If ~Exists(Source) Then Call ErrorMsg("Unable to find the newsgroup data directory.")
Source=Source||"/news_"

Address Command "Copy C:amiydec C:maildecoder to T:"
Say ''

UUDataString = xrange(d2c(32),d2c(99))||xrange(d2c(102),d2c(109))||xrange(d2c(111),d2c(126))

/***********************************************************/
/* Decode attachments from the currently selected messages */
/***********************************************************/

If DecodeMode = 2 Then Do
  Say "Finding marked messages..."
  Say ""
  Do I = 0 to NArticles-1
    ISMSGSELECTED I ; Selected.I = RC
  End
End

Do I = 0 to NArticles-1
  If DecodeMode = 2 & Selected.I = 0 Then Iterate
  SETMESSAGE I
  MESSAGEINFO ; Parse VAR Result MID .
  If MID = LastMID Then Leave
  LastMID = MID
  NewsFile = Source||MID
  BFlag = 0   ; CFlag = 0      ; DFlag = 0    ; MFlag = 0
  MimeID = "" ; MimeBound = "" ; MimePart = 0 ; MimeTotal = 0
  UUFlag = 0  ; yEnc = 0

  Call Open('In',NewsFile,"R")
    Header = "!"
    Do Until Header = ""
      Header = Strip(ReadLn('In'))
      If Header = "X-NewsCoaster-Flag-Online: yes" Then Do
        Close('In')
        MESSAGEFETCH QUIET
        If RC = 2 Then Iterate I
        Call Open('In',NewsFile,"R")
        Header = "!"
      End
      If Upper(Word(Header,1)) = "MIME-VERSION:" Then MFlag = 2
      If Pos('number=',Header) > 0 Then Parse Var Header 'number=' MimePart ';'
      If Pos('total=',Header) > 0 Then Parse Var Header 'total=' MimeTotal ';'
      If Pos('id="',Header) > 0 Then Parse Var Header 'id="' MimeID '"'
      If Pos('boundary="',Header) > 0 Then Parse Var Header 'boundary="' MimeBound '"'
    End

    Do Until EOF('In')
      Data = ReadLn('In')
      If Data = "" Then Iterate

      /*****************************/
      /* MIME Encapsulated Message */
      /*****************************/

      If MimeID ~= "" | MimeBound ~= "" | MimePart ~= 0 | MimeTotal ~= 0 | MFlag ~= 0 Then Do

        If MimePart = 0 Then Do
          Call Close('In')
          Call Decode("T:MailDecoder",NewsFile,0)
          Leave
        End

        If MimePart > 1 Then Do
          If Exists(SrcName) Then Do
            LastPart = Word(StateF(SrcName),9)
            If MimePart - 1 = LastPart Then Call Open('Out',SrcName,"A")
            Else Do
              Say "Error: Found part "||MimePart||", expected part "||Lastpart + 1||"."
              Leave
            End
          End
          Else Do
            Say "Error: Found part "||MimePart||" without part 1."
            Leave
          End
        End

        If MimePart = 1 Then Do
          FName = "" ; EncType = ""
          Do Until EOF('In')
            If Upper(Left(Data,8)) = "CONTENT-" Then Do
              If Pos("name=",Data) > 0 Then Parse Var Data 'name="'FName'"'
              If Pos("-Encoding: ",Data) > 0 Then EncType = Word(Data,2)
            End
            If FName ~= "" & (Upper(EncType) = "BASE64" | Upper(EncType) = "X-UUENCODE") Then Do
              SrcName = Dest||FName||".src"
              Call Open('Out',SrcName,"W")
              Call WriteLn('Out','Content-Type: application/octet-stream; name="'||FName||'"')
              Call WriteLn('Out','Content-Transfer-Encoding: 'EncType)
              Call WriteLn('Out','')
              MFlag = 1
            End
            If MFlag = 1 then Break
            Data = ReadLn('In')
          End
        End

        Say "Found part "||MimePart||"/"||MimeTotal||" of "||FName

        Do Until EOF('In')
          Data = ReadLn('In')
          If Upper(EncType) = "BASE64" Then Do
            If (Words(Data) = 1) & (Length(Data) % 4 = Length(Data) / 4) Then Call WriteLn('Out',Data)
          End
          If Upper(EncType) = "X-UUENCODE" Then Do
            If Upper(Word(Data,1)) = "BEGIN" & Datatype(Word(Data,2)) = "NUM" Then Call WriteLn('Out',Data)
            If Length(Data) = 0 | Words(Data) > 1 Then Iterate
            If Left(Data,3) = "end" Then Do
              Call WriteLn('Out',Data)
              Leave
            End
            DataSize=((C2D(Left(Data,1))-32)%.75)+1
            If DataSize = Length(Data) | Data = "`" | Data = "``" Then Call WriteLn('Out',Data)
          End
        End

        Call Close('Out')

        TempName = EscapeName(SrcName)
        Address Command 'Filenote "'||TempName||'" Comment "Lastpart '||MimePart||'"'

        If MimePart = MimeTotal Then Call Decode("T:MailDecoder",SrcName,1)

        Leave
      End

      /*********************/
      /* UUEncoded Message */
      /*********************/

      If Upper(Word(Data,1)) = "BEGIN" & Datatype(Word(Data,2)) = "NUM" Then Do
        FName = SubWord(Data,3)
        SrcName = Dest||FName||".src"
        Say "Found UUEncoded file => "FName
        A = Seek('In',0,"Current")
        B = Seek('In',-512,"End")
        EndData = ReadCh('In',512)
        EndData = compress(EndData,UUDataString)
        If Pos('0a'x||'end'||'0a'x,EndData) > 0 Then Do
          Call Decode("T:MailDecoder",NewsFile,0)
          Leave
        End
        Call Seek('In',A,"b")
        Call Open('Out',SrcName,'W')
        UUFlag = 1
      End
      If C2D(Left(Data,1)) > 32 & C2D(Left(Data,1)) < 78 & ((C2D(Left(Data,1))-32)%.75)+1 = Length(Data) & Words(Data) = 1 Then UUFlag = 1
      If UUFlag = 1 Then Do
        Call WriteLn('Out',Data)
        DFlag = 0
        Do Until DFlag = 1
          If EOF('In') Then Leave
          FData = ReadLn('In')
          If Length(FData) = 0 | Words(FData) > 1 Then Iterate
          If FData = "end" Then Do
            Call WriteLn('Out',FData)
            Call Close('Out')
            Call Decode("T:MailDecoder",SrcName,1)
            DFlag = 1
          End
          DataSize=((C2D(Left(FData,1))-32)%.75)+1
          If DataSize = Length(FData) | FData = "`" | FData = "``" Then Call WriteLn('Out',FData)
        End
      End

      /************************/
      /* yEnc Encoded Message */
      /************************/

      If Pos("=ybegin",Data) = 1 Then Do
        Parse VAR Data "=ybegin part="Part" "Blah" size="FSize" name="FName
        Data2 = ReadLn('In')
        Parse VAR Data2 "=ypart begin="PBegin" end="PEnd
        If Pos("part=",Data) = 0 | (Part = 1 & PEnd = FSize) Then Do
          Call Decode("T:AmiyDec",NewsFile,0)
          Leave
        End
        SrcName = Dest||FName||'.src'
        Say "Found part "Part" for "FName
        If Part = 1 & PEnd < FSize Then Do
          Call Open('Out',SrcName,"W")
          yEnc = 1
        End
        If Part > 1 Then Do
          If Exists(SrcName) Then Do
            LastPart = Word(StateF(SrcName),9)
            If Part - 1 = LastPart Then Do
              Call Open('Out',SrcName,"A")
              yEnc = 1
            End
            Else Say "Error: Found part "||Part||", expected part "||Lastpart + 1||"."
          End
          Else Say "Error: Found part "||Part||" without part 1."
        End
        If yEnc = 1 Then Do
          Call WriteLn('Out',Data)
          Call WriteLn('Out',Data2)
          Do Until EOF('In')
            Call WriteCh('Out',ReadCh('In',65535))
          End
          Call WriteLn('Out','0a'x)
          Call Close('Out')
          TempName = EscapeName(SrcName)
          Address Command 'Filenote "'||TempName||'" Comment "Lastpart '||Part||'"'
        End
        If PEnd = FSize & Exists(SrcName) Then Call Decode("T:AmiyDec",SrcName,1)
        Leave
      End
    End
  Call Close('In')
End
Address Command "delete T:amiydec T:maildecoder QUIET"
Call RTEZREQUEST("All fininshed",'_Alright','Bulk decode script','RTFI_FLAGS=EZREQF_CENTERTEXT')
Exit

Decode:
  Parse Arg Util,SrcFile,DelFlag
  Address Command Util||' "'||SrcFile||'"'
  If DelFlag = 1 Then Do
    TempName = EscapeName(SrcFile)
    Address Command 'Delete "'||TempName||'" Quiet'
  End
  Say ''
  Return

EscapeName:
  Parse Arg SrcFile
  TempName = ""
  If Length(SrcFile)~=Length(compress(SrcFile,'()[]~#?|')) Then Do
    Do PLoop = 1 to Length(SrcFile)
      w = SubStr(SrcFile,PLoop,1)
      If Pos(w,"()[]~#?|") > 0 Then w = "'"||w
      TempName = TempName||w
    End
  End
  Else TempName = SrcFile
  Return TempName


/************************************************************/
/* If there is an error, alert the user and exit the script */
/************************************************************/

ErrorMsg:
  Parse Arg ErrMsg
  Say "Fatal Error Encountered: "||ErrMsg
  Exit

