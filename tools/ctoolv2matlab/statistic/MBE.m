function finalResult = MBE(data)
sizeOfData=size(data);
result=0;
for j=1:sizeOfData(1,1)
    result=result+(data(j,2)-data(j,1));
end
finalResult=1/sizeOfData(1,1)*result;