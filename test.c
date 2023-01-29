#include <stdlib.h>
#include <stdio.h>
#include "extmem.h"

/**
* makeClear(Buffer *buf)
* ��buf����Ŀ�ȫ������Ϊ����
* buf: ������
*/
void makeClear(Buffer *buf){
    unsigned char *newblk;
    newblk = buf->data +1;
    for(int i = 0; i < 8; i++){
        freeBlockInBuffer(newblk, buf);
        newblk += buf->blkSize + 1;
    }
}

/**
* getTuple(unsigned char *blk, int index)
* �ú�������һ��Ԫ�飨X,Y������Ӧһ��Ԫ��
* newblk: ���ַ
* index : ������ĵڼ���Ԫ��,��0��7
*/
void getTuple(unsigned char *blk, int index, int a[2]){
    char str[5];
    for (int k = 0; k < 4; k++)
    {
        str[k] = *(blk + index*8 + k);
    }
    a[0] = atoi(str);
    for (int k = 0; k < 4; k++)
    {
        str[k] = *(blk + index*8 + 4 + k);
    }
    a[1] = atoi(str);
}

/**
*   wirteTuple(unsigned char *blk, int a[2], int index)
*   ��Ԫ��a[2]д�뵽blk����
*   nlk: Ҫд���blk
*   a: Ҫд���Ԫ��
*   index: д���λ��
*/
void writeTuple(unsigned char *blk, int *a, int index){
    char str1[5];
    char str2[5];
    // ��ʮ������ת�����ַ���
    itoa(a[0],str1,10);
    itoa(a[1],str2,10);
    for (int k = 0; k < 4; k++)
    {
        *(blk + index*8 + k)= str1[k];
        *(blk + index*8 + 4 + k)= str2[k];
    }
}

/**
* sort(int start, int end, Buffer *buf, int writeBackBlk, int writeToDisk)
* �Դ�������start��end�����������
* start: ��ʼ���̿�
* end :  �������̿�
* buf :  ������
* writeBackBlk: ������д��Ŀ�
* writeToDisk�� д�ش��̵Ŀ��
*/
void sort(int start, int end, Buffer *buf, int writeBackBlk, int writeToDisk){
    char str1[5];
    int blknum = start;
    int count = 0;
    unsigned char *newblk;
    int tuple1[2];
    int tuple2[2];
    int writeBack = writeBackBlk;
    // ����������
    while(blknum<=end)
    {
        if(count<7)
        {
            // �����̿��ȡ����������
            newblk = readBlockFromDisk(blknum,buf);
            count++;
            blknum++;
        }
        // ��������
        else{
            newblk = readBlockFromDisk(blknum,buf);
            blknum++;
            // ����������
            count = 0;
            // ��������(ð������)
            newblk = buf->data + 1;
            for(int k = 0; k< 64; k++){
                for(int i = 0; i < 62; i++){
                    // һ����ֻ���߸�Ԫ�飬�ڰ˸����������
                    if((i+1)%8!=0){
                        if((i+2)%8==0){
                            // ��ȡ��������Ԫ��
                            getTuple((newblk + (i/8)*(buf->blkSize) + i/8),i%8, tuple1);
                            getTuple((newblk + ((i+2)/8)*(buf->blkSize) + (i+2)/8), (i+2)%8, tuple2);
                            if(tuple1[0]>tuple2[0] || (tuple1[0]==tuple2[0]&&tuple1[1]>tuple2[1])){
                                writeTuple(newblk + (i/8)*(buf->blkSize) + i/8, tuple2, i%8);
                                writeTuple(newblk + ((i+2)/8)*(buf->blkSize) + (i+2)/8, tuple1, (i+2)%8);
                            }
                        }
                        else{
                            getTuple((newblk + (i/8)*(buf->blkSize) + i/8),i%8, tuple1);
                            getTuple((newblk + ((i+1)/8)*(buf->blkSize) + (i+1)/8), (i+1)%8, tuple2);
                            if(tuple1[0]>tuple2[0] || (tuple1[0] == tuple2[0]&&tuple1[1]>tuple2[1])){
                                writeTuple(newblk + (i/8)*(buf->blkSize) + i/8, tuple2, i%8);
                                writeTuple(newblk + ((i+1)/8)*(buf->blkSize) + (i+1)/8, tuple1, (i+1)%8);
                            }
                        }
                    }
                }
            }
            // ���������еĿ�д��������
            newblk = buf->data + 1;
            for(int i = 0; i < 8; i++){
                for (int k = 0; k < 4; k++)
                {
                    itoa(writeBack+1,str1,10);
                    *(newblk + 7*8 + k + i*buf->blkSize + i)= str1[k];
                    *(newblk + 7*8 + 4 + k + i*buf->blkSize + i)= '\0';
                }
                writeBlockToDisk((newblk + i*(buf->blkSize) + i),writeBack,buf);
                writeBack++;
            }
            // ���������еĿ�ȫ������Ϊ����
            makeClear(buf);
        }

    }

    // ���й鲢����
    makeClear(buf);
    unsigned char *wbblk;
    int tempWriteBackBlk = writeToDisk;
    int countx[8] = {0,0,0,0,0,0,0,0};
    int countx2[8] = {0,8,16,24,32,40,48,56};
    int numbers[8][2] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    int flags[8] = {0,0,0,0,0,0,0,0};
    int countBlks = 0;
    int tempStart = writeBackBlk;
    int tempEnd = writeBackBlk + end - start;
    int sets = (end-start + 1)/8;
    for(int i = 0; i < 8; i++){
        countx2[i]+=tempStart;
    }
    for(int i = 0; i < 8; i++){
        if(i>=sets){
            flags[i] = 1;
        }
    }
    unsigned char *tempBlk;
    wbblk = getNewBlockInBuffer(buf);
    int continueFlag = 0;
    // ���й鲢����
    while(countBlks<=tempEnd-tempStart){
        for(int i = 0; i < 8; i++){
            // �ж�ÿ��������Ԫ���Ƿ��Ѿ�ȡ��
            if(countx[i]==7){
                continueFlag = 1;
                if(i == 7){
                    countx[7] = 0;
                    countBlks++;
                    for (int k = 0; k < 4; k++)
                    {
                        itoa(tempWriteBackBlk+1,str1,10);
                        *(wbblk + 7*8 + k)= str1[k];
                        *(wbblk + 7*8 + 4 + k)= '\0';
                    }
                    writeBlockToDisk(wbblk, tempWriteBackBlk, buf);
                    tempWriteBackBlk++;
                    wbblk = getNewBlockInBuffer(buf);
                    continue;
                }
                else{
                    if(countx2[i]<tempStart+(i+1)*8-1){
                        countx2[i]++;
                    }
                    else{
                        flags[i] = 1;
                    }
                    countx[i] = 0;
                    continue;
                }
            }
        }
        if(continueFlag==1){
            continueFlag = 0;
            continue;
        }
        // ��ÿ�����ϵ���С����ȡ����
        for(int i = 0; i < sets; i++){
            if(flags[i]!=1){
                tempBlk = readBlockFromDisk(countx2[i],buf);
                getTuple(tempBlk, countx[i], numbers[i]);
                freeBlockInBuffer(tempBlk,buf);
            }
        }
        // ȡ����С��
        int min[2] = {99999,99999};
        int index = -1;
        for(int i = 0; i < sets; i++){
            if(!flags[i]){
                if(numbers[i][0]<min[0] || (numbers[i][0]==min[0]&&numbers[i][1]<min[1])){
                    min[0] = numbers[i][0];
                    min[1] = numbers[i][1];
                    index = i;
                }
            }
        }
        // ����С��д�뻺����
        writeTuple(wbblk,min,countx[7]);
        // ��������һλ
        countx[7]++;
        countx[index]++;
    }
    makeClear(buf);
}

int main(int argc, char **argv)
{
    Buffer buf; /* A buffer */
    unsigned char *blk; /* A pointer to a block */
    int i = 0;

    /* Initialize the buffer */
    if (!initBuffer(520, 64, &buf))
    {
        perror("Buffer Initialization Failed!\n");
        return -1;
    }

    /* Get a new block in the buffer */
    blk = getNewBlockInBuffer(&buf);

    /* Fill data into the block */
    for (i = 0; i < 8; i++)
        *(blk + i) = 'a' + i;

    /* Write the block to the hard disk */
    if (writeBlockToDisk(blk, 8888, &buf) != 0)
    {
        perror("Writing Block Failed!\n");
        return -1;
    }

    /* Read the block from the hard disk */
    if ((blk = readBlockFromDisk(1, &buf)) == NULL)
    {
        perror("Reading Block Failed!\n");
        return -1;
    }
    freeBlockInBuffer(blk,&buf);

    int X = -1;
    int Y = -1;

    printf("------------------------------------------------------\n");
    printf("�������������Ĺ�ϵѡ���㷨 S.C = 128\n");
    printf("------------------------------------------------------\n");


    int numio = 0;
    int numt = 0;
    int count = 0;
    int wblk = 100;
    int tuple[2];
    unsigned char *newblk;
    char str1[5];
    newblk = getNewBlockInBuffer(&buf);
    memset(newblk, 0, buf.blkSize * sizeof(unsigned char));
    // ѭ������ÿһ����
    for(i = 17; i < 49; i++)
    {
        if ((blk = readBlockFromDisk(i, &buf)) == NULL)
        {
            perror("Reading Block Failed!\n");
            return -1;
        }
        numio++;
        printf("�������ݿ�%d\n", i);
        for (int j = 0; j < 7; j++) //һ��blk��7��Ԫ���һ����ַ
        {
            getTuple(blk, j,tuple);
            X = tuple[0];
            Y = tuple[1];
            if(X==128)
            {
                printf("(X = %d, Y = %d)\n", X, Y);
                // ���ҵ���Ԫ��д�뻺����
                writeTuple(newblk,tuple,count);
                numt++;
                count++;
                // ������������ˣ��ͽ�������д�뵽������
                if(count==7)
                {
                    for (int k = 0; k < 4; k++)
                    {
                        itoa(wblk+1,str1,10);
                        *(newblk + count*8 + k)= str1[k];
                    }
                    if (writeBlockToDisk(newblk, wblk, &buf) != 0)
                    {
                        perror("Writing Block Failed!\n");
                        return -1;
                    }

                    freeBlockInBuffer(newblk,&buf);
                    newblk = getNewBlockInBuffer(&buf);
                    memset(newblk, 0, buf.blkSize * sizeof(unsigned char));
                    count = 0;
                    wblk++;
                    numio++;
                }
            }
        }
        freeBlockInBuffer(blk, &buf);
    }
    // ����Ԫ�ص���δ����newblkд�ص�����
    if(count<7&&count!=0)
    {
        // ��������һ��Ԫ����Ϊ��һ����Ŀ��
        for (int k = 0; k < 4; k++)
        {
            itoa(wblk+1,str1,10);
            *(newblk + 7*8 + k)= str1[k];
            *(newblk + 7*8 + 4 + k)= '\0';
        }
        // ��Ԫ���⣬�����ط���Ϊ�հ�
        for(int j = count;j<7;j++){
            for (int k = 0; k < 8; k++)
            {
                *(newblk + j*8 + k)= '\0';
            }
        }
        // д�ش���
        if (writeBlockToDisk(newblk, wblk, &buf) != 0)
        {
            perror("Writing Block Failed!\n");
            return -1;
        }
        freeBlockInBuffer(newblk,&buf);
        newblk = getNewBlockInBuffer(&buf);
        memset(newblk, 0, buf.blkSize * sizeof(unsigned char));
        count = 0;
        wblk++;
        numio++;
    }
    for(i=100;i<wblk;i++)
    {
        printf("ע�����д�����: %d\n",i);
    }
    printf("\n");
    printf("����ѡ��������Ԫ��һ����%d��\n",numt);
    printf("IO��д����һ��%d��\n",numio);
    printf("\n");

    printf("------------------------------------------------------\n");
    printf("���׶ζ�·�鲢�����㷨��TPMMS��\n");
    printf("------------------------------------------------------\n");
    // ���Ƚ����ݶ��뻺����
    freeBlockInBuffer(newblk, &buf);
    count = 0;
    // ��������
    sort(1, 16, &buf, 201, 301);
    sort(17, 48, &buf, 217, 317);
    printf("\n����������R:201-216  S:217-248\n");
    printf("\n�鲢�����������ս������R:301-316  S:317-348\n\n");

    printf("------------------------------------------------------\n");
    printf("���������Ĺ�ϵѡ���㷨 S.C = 128\n");
    printf("------------------------------------------------------\n");

    // ΪS���������ļ�
    int indexBlkStart = 417; // Ҫд�صĿ�ĳ�ʼ���
    int startBlk = 317;      // ��ȡ�ĳ�ʼ���
    int endBlk = 348;        // ��ȡ�����տ��
    unsigned char *wbblk;
    int noRepeatNum = 0;     // ��¼���ظ�����
    int tempTuple[2];
    int countw = 0;          // ��¼������д�ؿ��еļ�¼��
    // ΪS���������ļ�
    while(startBlk<=endBlk){
        makeClear(&buf);
        // ��ȡ��
        newblk = readBlockFromDisk(startBlk,&buf);
        wbblk = getNewBlockInBuffer(&buf);
        for(int i = 0; i < 7; i++){
            getTuple(newblk,i,tempTuple);
            // countw==7ʱ��д�ؿ��Ѿ����ˣ����Ҫд�ش���
            if(countw==7){
                for (int k = 0; k < 4; k++)
                {
                    itoa(indexBlkStart+1,str1,10);
                    *(wbblk + 7*8 + k)= str1[k];
                    *(wbblk + 7*8 + 4 + k)= '\0';
                }
                writeBlockToDisk(wbblk, indexBlkStart, &buf);
                indexBlkStart++;
                wbblk = getNewBlockInBuffer(&buf);
                countw=0;
            }
            // �жϸ�Ԫ���id�Ƿ�����ϸ�Ԫ��
            if(tempTuple[0]!=noRepeatNum){
                noRepeatNum = tempTuple[0];
                tempTuple[1] = startBlk;
                writeTuple(wbblk,tempTuple, countw);
                countw++;
            }

        }
        startBlk++;
        freeBlockInBuffer(newblk, &buf);
    }
    // ��δ���Ŀ�д�ص�����
    if(countw>0){
        for(int i = 0; i < 7; i++){
            if(i>=countw){
                for (int k = 0; k < 8; k++)
                {
                    *(wbblk + i*8 + k)= '\0';
                }
            }
        }
        for (int k = 0; k < 4; k++)
        {
            itoa(indexBlkStart+1,str1,10);
            *(wbblk + 7*8 + k)= str1[k];
            *(wbblk + 7*8 + 4 + k)= '\0';
        }
        writeBlockToDisk(wbblk, indexBlkStart, &buf);
    }


    // ���������ļ����в���
    makeClear(&buf);
    int searchIndex = 128;          // ��Ҫ���ҵ�id
    int indexStart = 417;           // �����ļ���ʼ��
    int indexEnd = indexBlkStart;   // �����ļ�������
    int needBlk = -1;               // ��¼��Ӧid�����ݿ����һ������
    int breakFlag = 0;
    numio =0;
    while(indexStart<=indexEnd){
        // ��ȡ��
        newblk = readBlockFromDisk(indexStart, &buf);
        numio++;
        printf("����������%d\n",indexStart);
        // �����飬��ѯ�Ƿ��ж�Ӧid��������¼
        for(int i = 0; i < 7; i++){
            getTuple(newblk, i, tuple);
            if(tuple[0]==searchIndex){
                needBlk = tuple[1];
                breakFlag = 1;
                break;
            }
            if(i ==6 ){
                printf("û������������Ԫ��\n");
            }
        }
        if(breakFlag){
            break;
        }
        indexStart++;
        freeBlockInBuffer(newblk, &buf);
    }

    // ���ݵõ��Ŀ�Ž��в���
    freeBlockInBuffer(newblk, &buf);
    int count1 = 0;                     // ��¼�Ƿ��ҵ��˶�Ӧ��id
    int count2 = 0;                     // ��¼�Ƿ��Ѿ�������Ӧid
    wbblk = getNewBlockInBuffer(&buf);
    count = 0;                          // ��¼wbblk�м�¼��
    int countnum = 0;                  // ��¼��Ҫ��ѯ��id�ļ�¼����
    int writeBackBlk = 501;            // ��ѯ���д�صĿ�ʼ��
    int writeBack = writeBackBlk;
    int need = needBlk;
    // ���ݵõ��Ŀ�Ž��в���
    while(!count2){
        printf("�������ݿ�%d\n", needBlk);
        newblk = readBlockFromDisk(needBlk, &buf);
        numio++;
        for(int i = 0; i < 7; i++){
            getTuple(newblk, i, tuple);
            if(count == 7){
                for (int k = 0; k < 4; k++)
                {
                    itoa(writeBackBlk+1,str1,10);
                    *(wbblk + 7*8 + k)= str1[k];
                    *(wbblk + 7*8 + 4 + k)= '\0';
                }
                count=0;
                writeBlockToDisk(wbblk, writeBackBlk, &buf);
                numio++;
                writeBackBlk++;
                wbblk = getNewBlockInBuffer(&buf);
            }
            if(tuple[0]==searchIndex){
                countnum++;
                printf("(X=%d, Y=%d)\n", tuple[0], tuple[1]);
                writeTuple(wbblk, tuple, count);
                count++;
                count1 = 1;
            }
            else{
                if(count1){
                    count2 = 1;
                }
            }
        }
        freeBlockInBuffer(newblk, &buf);
        needBlk++;
    }
    // ��δ���Ŀ�д�ص�����
    if(count>0){
        for(int i = 0; i < 7; i++){
            if(i>=count){
                for (int k = 0; k < 8; k++)
                {
                    *(wbblk + i*8 + k)= '\0';
                }
            }
        }
        for (int k = 0; k < 4; k++)
        {
            itoa(writeBackBlk+1,str1,10);
            *(wbblk + 7*8 + k)= str1[k];
            *(wbblk + 7*8 + 4 + k)= '\0';
        }
        writeBlockToDisk(wbblk, writeBackBlk, &buf);
        numio++;
    }
    // ��������Ϣ
    for(int i = writeBack; i <= writeBackBlk; i++){
        printf("\nע�����д����̣�%d\n", i);
    }
    printf("\n����ѡ��������Ԫ��һ��%d��\n", countnum);
    printf("\nIO��дһ��%d��\n", numio);


    printf("------------------------------------------------------\n");
    printf("��������������㷨\n");
    printf("------------------------------------------------------\n");
    // ����������R��S��
    makeClear(&buf);
    int rStartBlk = 301;
    int rEndBlk = 316;
    int sStartBlk = 317;
    int sEndBlk = 348;
    int tuple1[2],tuple2[2];
    int connectNum = 0;
    count1 = 0;
    count2 = 0;
    countw = 0;
    wbblk = getNewBlockInBuffer(&buf);
    unsigned char *rblk,*sblk;
    writeBack = 601;
    int backblk = rStartBlk;
    int backIndex = 0;
    int backFlag = 0;
    while(rStartBlk<=rEndBlk&&sStartBlk<=sEndBlk){

        rblk = readBlockFromDisk(rStartBlk, &buf);
        sblk = readBlockFromDisk(sStartBlk, &buf);
        // д�ؿ���������д������
        if(countw==3){
            countw=0;
            for (int k = 0; k < 4; k++)
            {
                itoa(writeBack+1,str1,10);
                *(wbblk + 6*8 + k)= '\0';
                *(wbblk + 6*8 + 4 + k)= '\0';
                *(wbblk + 7*8 + k)= str1[k];
                *(wbblk + 7*8 + 4 + k)= '\0';
            }
            writeBlockToDisk(wbblk, writeBack, &buf);
            printf("ע�����д����̣�%d\n",writeBack);
            wbblk = getNewBlockInBuffer(&buf);
            writeBack++;
        }
        // ���R�еĿ��е�Ԫ��ȫ���Ѿ���ȡ�꣬��ȡ��һ��
        if(count1==7){
            rStartBlk++;
            count1 = 0;
            freeBlockInBuffer(rblk, &buf);
            freeBlockInBuffer(sblk, &buf);
            continue;
        }
        // ���S�еĿ��е�Ԫ��ȫ���Ѿ����꣬��ȡ��һ��
        if(count2==7){
            sStartBlk++;
            count2 = 0;
            freeBlockInBuffer(rblk, &buf);
            freeBlockInBuffer(sblk, &buf);
            continue;
        }
        // ��ȡ��������Ԫ��
        getTuple(rblk, count1, tuple1);
        getTuple(sblk, count2, tuple2);
        // ���R����Ԫ�ص�����С�� S����Ԫ�ص���������ȡR������һ��Ԫ��
        if(tuple1[0]<tuple2[0]){
            if(backFlag){
                backFlag = 0;
                sStartBlk = backblk;
                count2 = backIndex;
            }
            count1++;
        }
        // ���R����Ԫ�ص��������� S����Ԫ�ص���������ȡS������һ��Ԫ��
        else if(tuple1[0]>tuple2[0]){
            if(backFlag){
                backFlag = 0;
                sStartBlk = backblk;
                count2 = backIndex;
            }
            else{
                count2++;
            }
        }
        // ���R����Ԫ�ص��������� S����Ԫ�ص�����
        // ��¼������š������� �����ݱ�־��Ϊ1
        else{
            if(!backFlag){
                backblk = sStartBlk;
                backIndex = count2;
                backFlag = 1;
            }
            writeTuple(wbblk, tuple1, countw*2);
            writeTuple(wbblk, tuple2, 1 + countw*2);
            countw++;
            connectNum++;
            count2++;
        }
        freeBlockInBuffer(rblk, &buf);
        freeBlockInBuffer(sblk, &buf);
    }
    // ��δ���Ŀ�д�ص�����
    if(countw>0){
        for(int i = 0; i < 7; i++){
            if(i>=countw*2){
                for (int k = 0; k < 8; k++)
                {
                    *(wbblk + i*8 + k)= '\0';
                }
            }
        }
        for (int k = 0; k < 4; k++)
        {
            itoa(writeBack+1,str1,10);
            *(wbblk + 7*8 + k)= str1[k];
            *(wbblk + 7*8 + 4 + k)= '\0';
        }
        writeBlockToDisk(wbblk, writeBack, &buf);
        printf("ע�����д����̣�%d\n",writeBack);
    }
    printf("�ܹ�����%d��\n", connectNum);
    makeClear(&buf);



    printf("------------------------------------------------------\n");
    printf("��������ļ��ϵĽ��㷨\n");
    printf("------------------------------------------------------\n");
    rStartBlk = 301;
    rEndBlk = 316;
    sStartBlk = 317;
    sEndBlk = 348;
    wbblk = getNewBlockInBuffer(&buf);
    countw = 0;
    count1 = 0;
    count2 = 0;
    writeBack = 1001;
    countnum = 0;
    while(rStartBlk<=rEndBlk&&sStartBlk<=sEndBlk){
        rblk = readBlockFromDisk(rStartBlk, &buf);
        sblk = readBlockFromDisk(sStartBlk, &buf);
        // д�ؿ���������д������
        if(countw==7){
            countw=0;
            for (int k = 0; k < 4; k++)
            {
                itoa(writeBack+1,str1,10);
                *(wbblk + 7*8 + k)= str1[k];
                *(wbblk + 7*8 + 4 + k)= '\0';
            }
            writeBlockToDisk(wbblk, writeBack, &buf);
            printf("ע�����д����̣�%d\n",writeBack);
            wbblk = getNewBlockInBuffer(&buf);
            writeBack++;
        }
        // ���R�еĿ��е�Ԫ��ȫ���Ѿ���ȡ�꣬��ȡ��һ��
        if(count1==7){
            rStartBlk++;
            count1 = 0;
            freeBlockInBuffer(rblk, &buf);
            freeBlockInBuffer(sblk, &buf);
            continue;
        }
        // ���S�еĿ��е�Ԫ��ȫ���Ѿ����꣬��ȡ��һ��
        if(count2==7){
            sStartBlk++;
            count2 = 0;
            freeBlockInBuffer(rblk, &buf);
            freeBlockInBuffer(sblk, &buf);
            continue;
        }
        // ��ȡ��������Ԫ��
        getTuple(rblk, count1, tuple1);
        getTuple(sblk, count2, tuple2);
        // ���R����Ԫ�ص�����С�� S����Ԫ�ص���������ȡR������һ��Ԫ��
        if(tuple1[0]<tuple2[0]){
            count1++;
        }
        // ���R����Ԫ�ص��������� S����Ԫ�ص���������ȡS������һ��Ԫ��
        else if(tuple1[0]>tuple2[0]){
            count2++;
        }
        // ���R����Ԫ�ص��������� S����Ԫ�ص�����
        else{
            if(tuple1[1]==tuple2[1]){
                printf("(X=%d, Y=%d)\n", tuple1[0], tuple1[1]);
                writeTuple(wbblk, tuple1, countw);
                countw++;
                count++;
                countnum++;
                count1++;
            }
            count2++;
        }
        freeBlockInBuffer(rblk, &buf);
        freeBlockInBuffer(sblk, &buf);
    }
    // ��δ���Ŀ�д�ص�����
    if(countw>0){
        for(int i = 0; i < 7; i++){
            if(i>=countw){
                for (int k = 0; k < 8; k++)
                {
                    *(wbblk + i*8 + k)= '\0';
                }
            }
        }
        for (int k = 0; k < 4; k++)
        {
            itoa(writeBack+1,str1,10);
            *(wbblk + 7*8 + k)= str1[k];
            *(wbblk + 7*8 + 4 + k)= '\0';
        }
        writeBlockToDisk(wbblk, writeBack, &buf);
        printf("ע�����д����̣�%d\n",writeBack);
    }
    printf("R��S�Ľ�����%d��Ԫ��\n", countnum);
    makeClear(&buf);
    return 0;
}

