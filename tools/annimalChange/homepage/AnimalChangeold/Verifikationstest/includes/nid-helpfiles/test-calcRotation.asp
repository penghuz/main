<!-- #include file="RotationModel.asp" -->

<%
'   NumberCrops = 3
   Dim A  ' array of crop areas NUMBERCROPS
   Redim A(3)
   Dim B  ' array of crop areas NUMBERCROPS
   Redim B(6)
   Dim ObjectCoeff   ' matrix of objective function coefficients NUMBERCROPS * NUMBERCROPS
                     ' -10000 indicates that it is an impossible combination of precrop and crop
   Redim ObjectCoeff(Ubound(A),Ubound(B))

   Dim arrRotationList
   Redim arrRotationList(Ubound(A),Ubound(B))
   
   A(0)=8
   A(1)=15
   A(2)=3
   B(0)=3
   B(1)=4
   B(2)=7
   B(3)=8
   B(4)=1
   B(5)=4
   B(6)=8
   A(3)=B(0)+B(1)+B(2)+B(3)-A(0)-A(1)-A(2)+B(4)+B(5)+B(6)


   ObjectCoeff(0,0)=7
   ObjectCoeff(1,0)=44
   ObjectCoeff(2,0)=46
   ObjectCoeff(3,0)=47
   ObjectCoeff(0,1)=60
   ObjectCoeff(1,1)=62
   ObjectCoeff(2,1)=7
   ObjectCoeff(3,1)=64
   ObjectCoeff(0,2)=40
   ObjectCoeff(1,2)=41
   ObjectCoeff(2,2)=49
   ObjectCoeff(3,2)=43
   ObjectCoeff(0,3)=50
   ObjectCoeff(1,3)=55
   ObjectCoeff(2,3)=45
   ObjectCoeff(3,3)=60
   ObjectCoeff(0,4)=2
   ObjectCoeff(1,4)=48
   ObjectCoeff(2,4)=-10000
   ObjectCoeff(3,4)=41
   ObjectCoeff(0,5)=-10000
   ObjectCoeff(1,5)=52
   ObjectCoeff(2,5)=23
   ObjectCoeff(3,5)=67
   ObjectCoeff(0,6)=42
   ObjectCoeff(1,6)=48
   ObjectCoeff(2,6)=2
   ObjectCoeff(3,6)=41

   InitArray arrRotationList,-1

   PrintArrayInTable(ObjectCoeff)
   PrintArrayInTable1(A)
   PrintArrayInTable1(B)

   test=calcRotation(A,B,ObjectCoeff,arrRotationList)
   
   Response.write "Test = "&test &"<br>"
   
   PrintArrayInTable(arrRotationList)

%>