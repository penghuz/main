<%
'**************************************************************************************************
' Author: Poul Lassen, Danish Institute of Agricultural Sciences
' Description: 
'**************************************************************************************************
sub CreateTabField()
   dim r,f
   dim nRotation, arrRotation, aRotationID
   dim nFieldPlan, arrFieldPlan
   dim nFeedStuff, arrFeedStuff, aFeedStuffID, dictFeedStuff
   dim nMainFeedStuff, arrMainFeedStuff, aMainFeedStuffID, dictMainFeedStuff
   dim nStrawFeedStuff, arrStrawFeedStuff, aStrawFeedStuffID, dictStrawFeedStuff
   dim nSoilType, arrSoilType, aSoilTypeID
   dim nIrrigationType, arrIrrigationType, aIrrigationType
   dim nCrop, arrCrop, aCropID
   dim nAfterCrop, arrAfterCrop, aAfterCropID, canHaveAfterCrop, aUseStraw
   dim aStartDate, aEndDate, strDateCheck, strStartDateCheck, strEndDateCheck 
   dim nYieldLevel, aYieldLevel
   dim aTotalArea, rotationSpan
   dim nScenarioData, arrScenarioData, dictScenarioData
   dim currentLocale
   
   currentLocale = getLocale()
   setLocale("da")
   DefineValidateScript
   'arrYieldLevel, aYieldLevel
 time6=now  
   InitFarmNDB
   CreateLanguageDictionary
   Dim dictRotation
   set dictRotation = CreateObject("Scripting.Dictionary")
   nRotation = CallSP ("GetRotationList " & Session("farmNumber")& "," & Session("scenarioNumber") , arrRotation, dictRotation)
time1=now
timeMeasureString1 = "After GetRotationList :"&cstr(time1)
'   nRotation = GetRotationList(CLng(Session("farmNumber")),CInt(Session("scenarioNumber")),arrRotation)
   Dim dictSoilType
   set dictSoilType = CreateObject("Scripting.Dictionary")
   nSoilType = CallSP ("GetSelectListSoilType" , arrSoilType, dictSoilType)
'   nSoilType = GetSelectListSoilType(arrSoilType)
time1=now
timeMeasureString2 = "After GetSelectListSoilType :"&cstr(time1)
   Dim dictCrop
   set dictCrop = CreateObject("Scripting.Dictionary")
   nCrop = CallSP ("GetSelectListCrop" , arrCrop, dictCrop)
'   nCrop     = GetSelectListCrop(arrCrop)
time1=now
timeMeasureString1 = "After GetSelectListCrop :"&cstr(time1)
   Dim dictAfterCrop
   set dictAfterCrop = CreateObject("Scripting.Dictionary")
   nAfterCrop = CallSP ("GetSelectListAfterCrop" , arrAfterCrop, dictAfterCrop)
time1=now
timeMeasureString3 = "After GetSelectListAfterCrop :"&cstr(time1)

   set dictFeedStuff = CreateObject("Scripting.Dictionary")
   nFeedStuff = CallSP ("GetFeedStuffList" , arrFeedStuff, dictFeedStuff)
time1=now
timeMeasureString4 = "After GetFeedStuffList :"&cstr(time1)
   
   set dictScenarioData = CreateObject("Scripting.Dictionary")
   nScenarioData = CallSP ("GetScenarioData " & CLng(Session("farmNumber"))& "," & Session("scenarioNumber") , arrScenarioData, dictScenarioData)	   
   'response.write nFeedStuff
time20=now
   redim arrIrrigationType(1,1)
   nIrrigationType 			= 2
   arrIrrigationType(0,0)	= 0
   arrIrrigationType(1,0)	= Str.Item("strRAINFED")
   arrIrrigationType(0,1)	= 1
   arrIrrigationType(1,1)	= Str.Item("strIRRIGATED")

   Response.Write("<script language=""javascript"">window.parent.frames[1].location.reload()</script>" & vbCr)
'   Response.Write("<span id=""veil"" style=""position: absolute; top: 0px; left: 0px; height: 100%; width: 100%; background-color: #C8E1F3; display: none;""><blink style=""position: relative; top: 50%; left: 50%;"">In progress...</blink></span>" & vbCr)
   Response.Write("<form name=""formFieldPlan"" action="""" method=""post"">" & vbCr)
   if nRotation < 1  then
	   Response.Write("  <input type=""checkbox"" name=""landless"" onchange=""document.forms.update.saveUpdates.value='true';document.forms.formFieldPlan.action='LandlessUpdate.asp';document.forms.formFieldPlan.submit();"" value="""&arrScenarioData(0,dictScenarioData.item("Landless"))&"""")
	   if  arrScenarioData(0,dictScenarioData.item("Landless")) = "True" then
	   Response.Write(" checked />"&Str.Item("strLandless")&"" & vbCr)
	   Response.Write("</form>" & vbCr)
       CreateUpdateFormFieldPlan
	   Response.Write("</body></html>" & vbCr)
	   Response.end
	   else
	   Response.Write("/>"&Str.Item("strLandless")&"" & vbCr)
	'   response.write arrScenarioData(0,dictScenarioData.item("Landless"))
	   end if
   end if
   Response.Write("  <table class=""data"" border=""0"">" & vbCr)
  
   Dim dictFieldPlan

   for r = 0 to nRotation-1
      aRotationID = arrRotation(r,dictRotation.item("RotationID"))
      aSoilTypeID = arrRotation(r,dictRotation.item("SoilType"))
	  aIrrigationType = arrRotation(r, dictRotation.item("Irrigation"))
	  if aIrrigationType="True" then
	     aIrrigationType=1
	  else
	     aIrrigationType=0
	  end if
'	  response.write aIrrigationType
	  aYieldLevel = arrRotation(r,dictRotation.item("YieldLevel"))	  
      Response.Write("    <tr>" & vbCr)
      Response.Write("      <td align=""left"" colspan=""2""><h4>"&Str.Item("strRotationName")&": </h4>" & vbCr)
	  Response.Write("<input type=""text"" name=""RotationName_"&aRotationID&""" value="""&arrRotation(r,dictRotation.item("RotationName"))&""" maxlength=""50"" size=""30"" onChange=""document.forms.update.saveUpdates.value='true';document.forms.formFieldPlan.action='FieldPlanUpdate.asp?RotationID="&aRotationID&"';document.forms.update.action='javascript:document.forms.formFieldPlan.submit();';this.style.color='#ff0000';""/>" & vbCr)
	  Response.Write("</td>" & vbCr)
      Response.Write("      <td align=""left"" colspan=""2""><h4>"&Str.Item("strSOILTYPE")&" </h4>" & vbCr)
	  Response.Write("<select name=""SoilType_"&aRotationID&""" onChange=""if ((this.options[this.selectedIndex].value=='1')||(this.options[this.selectedIndex].value=='2')||(this.options[this.selectedIndex].value=='3')||(this.options[this.selectedIndex].value=='4')){document.forms.formFieldPlan.Irrigation_"&aRotationID&".disabled=false;}else{document.forms.formFieldPlan.Irrigation_"&aRotationID&".options[0].selected=true;document.forms.formFieldPlan.Irrigation_"&aRotationID&".disabled=true;};document.forms.update.saveUpdates.value='true';document.forms.formFieldPlan.action='FieldPlanUpdate.asp?RotationID="&aRotationID&"&Irrigation_"&aRotationID&"=0';document.forms.update.action='javascript:document.forms.formFieldPlan.submit();';this.style.color='#ff0000';"">" & vbCr)
      CreateSelectList_SoilType aSoilTypeID,nSoilType,arrSoilType
	  Response.Write("</select></td>" & vbCr)
      Response.Write("      <td align=""left""><h4>"&Str.Item("strIRRIGATION")&" </h4>" & vbCr)
   set dictFieldPlan = CreateObject("Scripting.Dictionary")
   nFieldPlan = CallSP ("GetFieldPlanList " & CLng(Session("farmNumber")) & "," & CInt(Session("scenarioNumber")) & "," & aRotationID, arrFieldPlan, dictFieldPlan)
'      for f = 0 to nFieldPlan-1
		 aTotalArea=aTotalArea+Cdbl(arrFieldPlan(0,dictFieldPlan.item("Area")))
'      next
    rotationSpan=DateDiff("yyyy",Cdate(Cstr(arrFieldPlan(0,dictFieldPlan.item("StartDate")))),Cdate(Cstr(arrFieldPlan(nFieldPlan-1,dictFieldPlan.item("EndDate")))))
	if rotationSpan <= 0 then rotationSpan = 1
		 aTotalArea=Cdbl(arrFieldPlan(0,dictFieldPlan.item("Area")))/rotationSpan
   select case aSoilTypeID
   case 1,2,3,4
	  Response.Write("<select name=""Irrigation_"&aRotationID&""" onChange=""document.forms.update.saveUpdates.value='true';document.forms.formFieldPlan.action='FieldPlanUpdate.asp?RotationID="&aRotationID&"';document.forms.update.action='javascript:document.forms.formFieldPlan.submit();';this.style.color='#ff0000';"">" & vbCr)
   case else
	  Response.Write("<select name=""Irrigation_"&aRotationID&""" onChange=""document.forms.update.saveUpdates.value='true';document.forms.formFieldPlan.action='FieldPlanUpdate.asp?RotationID="&aRotationID&"';document.forms.update.action='javascript:document.forms.formFieldPlan.submit();';this.style.color='#ff0000';"" disabled>" & vbCr)
   end select
      CreateSelectList aIrrigationType,nIrrigationType,arrIrrigationType
	  Response.Write("</select></td>" & vbCr)
	  Response.Write("      <td align=""left""><h4>"&Str.Item("strAREA")&": </h4><input type=""text"" name=""inputArea_"&aRotationID&""" value="""&Round(aTotalArea,1)&""" maxlength=""5"" size=""8"" onChange=""if (isFinite(this.value.replace(',','.'))) {this.value=this.value.replace('.',',');document.forms.update.saveUpdates.value='true';document.forms.formFieldPlan.action='FieldPlanUpdate.asp?RotationID="&aRotationID&"';document.forms.update.action='javascript:document.forms.formFieldPlan.submit();';this.style.color='#ff0000';}""/>Ha</td>" & vbCr)
'      Response.Write("      <td align=""left""><h4>"&Str.Item("strYIELDLEVEL")&": " & vbCr)
'	  Response.write(arrRotation(r,dictRotation.item("YieldLevel"))&"%")   
      Response.Write("<input type = ""hidden"" name=""YieldLevel_"&aRotationID&""" value="""&arrRotation(r,dictRotation.item("YieldLevel"))&""" />")
'	  Response.Write("</h4></td>" & vbCr)
	  Response.Write("    </tr>" & vbCr)
      ' column headings
      Response.Write("    <tr>" & vbCr)   
      Response.Write("      <th align=""left"">"&Str.Item("strCROP")&"</th>" & vbCr)
      Response.Write("      <th align=""left"">"&Str.Item("strFeedstuff")&"</th>" & vbCr)
      Response.Write("      <th align=""left"">Straw "&Str.Item("strFeedstuff")&"</th>" & vbCr)
'      Response.Write("      <th align=""left"">"&Str.Item("strAREAHA")&"</th>" & vbCr)
      Response.Write("      <th align=""left"">Start date&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;End date</th>" & vbCr)'"&Str.Item("strAfterCrop")&"</th>" & vbCr)
      Response.Write("      <th align=""left"">&nbsp;</th>" & vbCr)
      Response.Write("    </tr>" & vbCr)
      
'   set dictFieldPlan = CreateObject("Scripting.Dictionary")
'   nFieldPlan = CallSP ("GetFieldPlanList " & CLng(Session("farmNumber")) & "," & CInt(Session("scenarioNumber")) & "," & aRotationID, arrFieldPlan, dictFieldPlan)
  '	  nFieldPlan = GetFieldPlanList(CLng(Session("farmNumber")),CInt(Session("scenarioNumber")),aRotationID,arrFieldPlan)
      for f = 0 to nFieldPlan-1
         aCropID = arrFieldPlan(f,dictFieldPlan.item("Crop"))
				 aAfterCropID = arrFieldPlan(f,dictFieldPlan.item("AfterCropID"))
				 canHaveAfterCrop = cBool(arrFieldPlan(f,dictFieldPlan.item("Can_Have_AfterCrop")))
				 aFeedStuffID=arrFieldPlan(f,dictFieldPlan.item("FeedStuffID"))
				 aStrawFeedStuffID=arrFieldPlan(f,dictFieldPlan.item("StrawFeedStuffID"))
				 aUseStraw = cBool(arrFieldPlan(f,dictFieldPlan.item("UseStraw")))
				 aStartDate = Cstr(arrFieldPlan(f,dictFieldPlan.item("StartDate")))
				 if aStartDate = "0" then aStartDate="" 
				 aEndDate = Cstr(arrFieldPlan(f,dictFieldPlan.item("EndDate")))
				 if aEndDate = "0" then aEndDate="" 
   set dictMainFeedStuff = CreateObject("Scripting.Dictionary")
   nMainFeedStuff = CallSP ("GetCropMainFeedstuffList "&aCropID , arrMainFeedStuff, dictMainFeedStuff)
   set dictStrawFeedStuff = CreateObject("Scripting.Dictionary")
   nStrawFeedStuff = CallSP ("GetCropStrawFeedstuffList "&aCropID , arrStrawFeedStuff, dictStrawFeedStuff)
         Response.Write("    <tr>" & vbCr)
         ' column 1 = crop
         Response.Write("      <td class=""data"" align=""left"">")
         Response.Write("<input type=""hidden"" name=""hiddenFieldPlanID_"&aRotationID&"_"&f&""" value="""&arrFieldPlan(f,dictFieldPlan.item("FieldPlanID"))&""">")
         Response.Write("<input type=""hidden"" name=""hiddenCropID_"&aRotationID&"_"&f&""" value="""&arrFieldPlan(f,dictFieldPlan.item("Crop"))&""">")
         Response.Write("<select name=""selectCrop_"&aRotationID&"_"&f&""" onChange=""veil();document.forms.update.saveUpdates.value='true';document.forms.formFieldPlan.action='FieldPlanUpdate.asp?RotationID="&aRotationID&"';document.forms.update.action='javascript:document.forms.formFieldPlan.submit();';this.style.color='#ff0000';submit();"">")   
         CreateSelectList_Language aCropID,nCrop,arrCrop
         Response.Write("</select>")   
         Response.Write("</td>" & vbCr)
         Response.Write("      <td class=""data"" align=""left"">")
         if cint(aCropID) <> 31 then
         Response.Write("<select style=""width: 280px"" name=""selectCropFeedStuff_"&aRotationID&"_"&f&""" onChange=""document.forms.update.saveUpdates.value='true';document.forms.formFieldPlan.action='FieldPlanUpdate.asp?RotationID="&aRotationID&"';document.forms.update.action='javascript:document.forms.formFieldPlan.submit();';this.style.color='#ff0000';"">")   
'         CreateSelectList_Language aCropID,nCrop,arrCrop
         CreateSelectList_ aFeedStuffID,nMainFeedStuff,arrMainFeedStuff
         Response.Write("</select>")   
		 end if
         Response.Write("</td>" & vbCr)
         Response.Write("      <td class=""data"" align=""left"">")
		 if aUseStraw = true then
         Response.Write("<select style=""width: 150px"" name=""selectStrawFeedStuff_"&aRotationID&"_"&f&""" onChange=""document.forms.update.saveUpdates.value='true';document.forms.formFieldPlan.action='FieldPlanUpdate.asp?RotationID="&aRotationID&"';document.forms.update.action='javascript:document.forms.formFieldPlan.submit();';this.style.color='#ff0000';"">")   
'         CreateSelectList_Language aCropID,nCrop,arrCrop
         CreateSelectList_ aStrawFeedStuffID,nStrawFeedStuff,arrStrawFeedStuff
         Response.Write("</select>")
		 end if   
         Response.Write("</td>" & vbCr)
         ' column 2 = area
'         Response.Write("      <td class=""data"" align=""left""><input type=""hidden"" name=""inputArea_"&aRotationID&"_"&f&""" value="""&arrFieldPlan(f,dictFieldPlan.item("Area"))&""" />"&arrFieldPlan(f,dictFieldPlan.item("Area"))&"</td>" & vbCr)
         Response.Write("      <input type=""hidden"" name=""inputArea_"&aRotationID&"_"&f&""" value="""&arrFieldPlan(f,dictFieldPlan.item("Area"))&""" />" & vbCr)
         Response.Write("      <td class=""data"" align=""left"">")   
         ' column 3 = StartDate - EndDate
		 if f>0 then
		   strStartDateCheck="if (document.getElementById('endDate_"&aRotationID&"_"&f-1&"').value !='') {if (ValidateDate(this,'Start date',false)) {var prevarr = (document.getElementById('endDate_"&aRotationID&"_"&f-1&"').value).split('-');var thisarr=(this.value).split('-'); if (thisarr.length < 2) {thisarr=(this.value).split('/')};if (parseInt(Date.UTC(thisarr[2],thisarr[1],thisarr[0]))-parseInt(Date.UTC(prevarr[2],prevarr[1],prevarr[0]))>=0) {document.forms.update.saveUpdates.value='true';this.style.color='#ff0000'} else {alert('Date must be on or after '+prevarr[0]+'-'+prevarr[1]+'-'+prevarr[2]);this.value='';}} else {this.value='';}} else {this.value='';alert('Enter start- and stop-dates from the top')};"
		 else
		   strStartDateCheck = "if (ValidateDate(this,'Start date',false))  {document.forms.update.saveUpdates.value='true';this.style.color='#ff0000';} else {this.value='';};"
		 end if

		   strEndDateCheck="if (document.getElementById('startDate_"&aRotationID&"_"&f&"').value !='') {if (ValidateDate(this,'End date',false)) {var prevarr = (document.getElementById('startDate_"&aRotationID&"_"&f&"').value).split('-');var thisarr=(this.value).split('-'); if (thisarr.length < 2) {thisarr=(this.value).split('/')};if (parseInt(Date.UTC(thisarr[2],thisarr[1],thisarr[0]))-parseInt(Date.UTC(prevarr[2],prevarr[1],prevarr[0]))>=0) {document.forms.update.saveUpdates.value='true';this.style.color='#ff0000'} else {alert('Date must be on or after '+prevarr[0]+'-'+prevarr[1]+'-'+prevarr[2]);this.value='';}} else {this.value='';}} else {this.value='';alert('Enter start- and stop-dates from the top')};"

         Response.Write("<input type=""text"" size=""9"" name=""startDate_"&aRotationID&"_"&f&""" id=""startDate_"&aRotationID&"_"&f&""" value="""&aStartDate&""" onchange="""&strStartDateCheck&""">&nbsp;&nbsp;")
         Response.Write("<input type=""text"" size=""9"" name=""endDate_"&aRotationID&"_"&f&""" id=""endDate_"&aRotationID&"_"&f&""" value="""&aEndDate&""" onchange="""&strEndDateCheck&""">")
           Response.Write("<input type=""hidden"" name=""selectAfterCrop_"&aRotationID&"_"&f&""" value=""0"">")   
         Response.Write("</td>" & vbCr)

'         Response.Write("      <td class=""data"" align=""left"">")   
'         ' column 3 = AfterCrop
'				 if canHaveAfterCrop = true then   
'           Response.Write("<select name=""selectAfterCrop_"&aRotationID&"_"&f&""" onChange=""document.forms.update.saveUpdates.value='true';document.forms.formFieldPlan.action='FieldPlanUpdate.asp?RotationID="&aRotationID&"';document.forms.update.action='javascript:document.forms.formFieldPlan.submit();';this.style.color='#ff0000';"">")   
'           CreateSelectList_Language aAfterCropID,nAfterCrop,arrAfterCrop
'           Response.Write("</select>")
'				 else
'				 end if         
'         Response.Write("</td>" & vbCr)

         Response.Write("      <td class=""data"" align=""left"">")   
  
         ' column 4 = delete buttons   
         Response.Write("&nbsp;&nbsp;<button style=""width: 125px; height: 20px; font-size: 10px"" onClick=""javascript:document.forms.update.saveUpdates.value='true';if(confirm('"&Str.Item("strBekræftafgrøde")&"')) {veil();document.forms.formFieldPlan.action='FieldPlanDelete.asp?RotationID="&aRotationID&"&FieldPlanID="&arrFieldPlan(f,dictFieldPlan.item("FieldPlanID"))&"';document.forms.formFieldPlan.submit();}"">"&Str.Item("strDELETECROP")&"</button>")
         Response.Write("</td>" & vbCr)   
         Response.Write("    </tr>" & vbCr)
		 set dictMainFeedStuff = nothing
		 set dictStrawFeedStuff = nothing
      next
      set dictFieldPlan = Nothing
      
      Response.Write("    <tr>" & vbCr)
      Response.Write("      <td class=""data"" colspan=""3"">")   
      Response.Write("<button style=""width: 125px; height: 20px; font-size: 10px"" onClick=""javascript:if (checkStartEndDate("&aRotationID&","&f&")) {document.forms.update.saveUpdates.value='false';} else {veil();document.forms.update.saveUpdates.value='false';document.forms.formFieldPlan.action='FieldPlanNew.asp?RotationID="&aRotationID&"';document.forms.formFieldPlan.submit();}"">"&Str.Item("strNEWCROP")&"</button>")
      Response.Write("&nbsp;")   
      Response.Write("<button style=""width: 125px; height: 20px; font-size: 10px"" onClick=""javascript:if (checkStartEndDate("&aRotationID&","&f&")) {document.forms.update.saveUpdates.value='false';} else {veil();document.forms.formFieldPlan.action='FieldPlanUpdate.asp?RotationID="&aRotationID&"';document.forms.formFieldPlan.submit();};"">"&Str.Item("strSAVEROT")&"</button>")
'      Response.Write("<button style=""width: 125px; height: 20px; font-size: 10px"" onClick=""javascript:document.forms.formFieldPlan.action='FieldPlanUpdate.asp?RotationID="&aRotationID&"';document.forms.formFieldPlan.submit();"">"&Str.Item("strSAVEROT")&"</button>")
      Response.Write("&nbsp;")   
      Response.Write("<button style=""width: 125px; height: 20px; font-size: 10px"" onClick=""javascript:document.forms.update.saveUpdates.value='false';if(confirm('"&Str.Item("strBekræftsædskifte")&"')) {veil();document.forms.formFieldPlan.action='RotationDelete.asp?RotationID="&aRotationID&"';document.forms.formFieldPlan.submit();}"">"&Str.Item("strDELROT")&"</button>")
      Response.Write("<input type=""hidden"" name=""hiddenNFieldPlan_"&aRotationID&""" value="""&nFieldPlan&""">")
      Response.Write("</td>" & vbCr)   
      Response.Write("    </tr>" & vbCr)

      Response.Write("    <tr>" & vbCr)
      Response.Write("      <td colspan=""6""><hr/></td>" & vbCr)
      Response.Write("    </tr>" & vbCr)
	  aTotalArea = 0
   next
   
   Response.Write("    <tr>" & vbCr)
   Response.Write("      <td colspan=""8"">")   
   Response.Write("<button style=""width: 200px; height: 20px; font-size: 10px"" onClick=""javascript:veil();document.forms.update.saveUpdates.value='false';document.forms.formFieldPlan.action='RotationNew.asp';document.forms.formFieldPlan.submit();"">"&Str.Item("strNEWROT")&"</button>")
   Response.Write("</td>" & vbCr)   
   Response.Write("    </tr>" & vbCr)
   CloseFarmNDB

   Response.Write("  </table>" & vbCr) 
   Response.Write("</form>" & vbCr)
   CreateUpdateFormFieldPlan
   time7=now
   setLocale(currentLocale)

end sub
'**************************************************************************************************
sub CreateUpdateFormFieldPlan()
'   Response.Write("<form name=""update"" action=""javascript:document.forms.formFieldPlan.submit();"" method=""post"">" & vbCr)
   Response.Write("<form name=""update"">" & vbCr)
   Response.Write("<input type=""hidden"" name=""saveUpdates"" value=""false"">" & vbCr)
   Response.Write("</form>" & vbCr)
end sub
'**************************************************************************************************
sub DefineValidateScript()
%><script language="javascript" type="text/javascript">
function checkStartEndDate(r,f)
{var ret = false;
for (var i = 0;i < f; i++)
{
  if (document.getElementById('startDate_'+r+'_'+i).value==''||document.getElementById('endDate_'+r+'_'+i).value=='')
  {
  ret = true;
//  break;
  }
  else
  {
  ret = false;
//  break;
  }
}
if (ret == true)
  {alert('fill out all start- and end-dates');}
return ret;
}
</script>
<%
end sub
'**************************************************************************************************
%>

