#include <stdlib.h>
#include <stdio.h>
#include "extmem.h"

/**
* makeClear(Buffer *buf)
* 将buf里面的块全部都置为空闲
* buf: 缓冲区
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
* 该函数返回一个元组（X,Y），对应一个元素
* newblk: 块地址
* index : 块里面的第几个元组,从0到7
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
*   将元组a[2]写入到blk里面
*   nlk: 要写入的blk
*   a: 要写入的元组
*   index: 写入的位置
*/
void writeTuple(unsigned char *blk, int *a, int index){
    char str1[5];
    char str2[5];
    // 将十进制数转换成字符串
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
* 对磁盘上面start到end块进行内排序
* start: 开始磁盘块
* end :  结束磁盘块
* buf :  缓冲区
* writeBackBlk: 内排序写会的块
* writeToDisk： 写回磁盘的块号
*/
void sort(int start, int end, Buffer *buf, int writeBackBlk, int writeToDisk){
    char str1[5];
    int blknum = start;
    int count = 0;
    unsigned char *newblk;
    int tuple1[2];
    int tuple2[2];
    int writeBack = writeBackBlk;
    // 进行内排序
    while(blknum<=end)
    {
        if(count<7)
        {
            // 将磁盘块读取到缓冲区中
            newblk = readBlockFromDisk(blknum,buf);
            count++;
            blknum++;
        }
        // 进行排序
        else{
            newblk = readBlockFromDisk(blknum,buf);
            blknum++;
            // 计数器清零
            count = 0;
            // 进行排序(冒泡排序)
            newblk = buf->data + 1;
            for(int k = 0; k< 64; k++){
                for(int i = 0; i < 62; i++){
                    // 一个块只有七个元组，第八个块参与排序
                    if((i+1)%8!=0){
                        if((i+2)%8==0){
                            // 获取相邻两个元组
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
            // 将缓冲区中的块写到磁盘上
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
            // 将缓冲区中的块全部都置为空闲
            makeClear(buf);
        }

    }

    // 进行归并排序
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
    // 进行归并排序
    while(countBlks<=tempEnd-tempStart){
        for(int i = 0; i < 8; i++){
            // 判断每个集合内元素是否已经取完
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
        // 将每个集合的最小数提取出来
        for(int i = 0; i < sets; i++){
            if(flags[i]!=1){
                tempBlk = readBlockFromDisk(countx2[i],buf);
                getTuple(tempBlk, countx[i], numbers[i]);
                freeBlockInBuffer(tempBlk,buf);
            }
        }
        // 取出最小数
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
        // 将最小数写入缓冲区
        writeTuple(wbblk,min,countx[7]);
        // 索引后移一位
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
    printf("基于线性搜索的关系选择算法 S.C = 128\n");
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
    // 循环遍历每一个块
    for(i = 17; i < 49; i++)
    {
        if ((blk = readBlockFromDisk(i, &buf)) == NULL)
        {
            perror("Reading Block Failed!\n");
            return -1;
        }
        numio++;
        printf("读入数据块%d\n", i);
        for (int j = 0; j < 7; j++) //一个blk存7个元组加一个地址
        {
            getTuple(blk, j,tuple);
            X = tuple[0];
            Y = tuple[1];
            if(X==128)
            {
                printf("(X = %d, Y = %d)\n", X, Y);
                // 将找到的元组写入缓冲区
                writeTuple(newblk,tuple,count);
                numt++;
                count++;
                // 如果缓冲区满了，就将缓冲区写入到磁盘中
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
    // 将有元素但是未满的newblk写回到磁盘
    if(count<7&&count!=0)
    {
        // 将块的最后一个元组置为下一个块的块号
        for (int k = 0; k < 4; k++)
        {
            itoa(wblk+1,str1,10);
            *(newblk + 7*8 + k)= str1[k];
            *(newblk + 7*8 + 4 + k)= '\0';
        }
        // 除元组外，其他地方置为空白
        for(int j = count;j<7;j++){
            for (int k = 0; k < 8; k++)
            {
                *(newblk + j*8 + k)= '\0';
            }
        }
        // 写回磁盘
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
        printf("注：结果写入磁盘: %d\n",i);
    }
    printf("\n");
    printf("满足选择条件的元组一共有%d个\n",numt);
    printf("IO读写次数一共%d次\n",numio);
    printf("\n");

    printf("------------------------------------------------------\n");
    printf("两阶段多路归并排序算法（TPMMS）\n");
    printf("------------------------------------------------------\n");
    // 首先将数据读入缓冲区
    freeBlockInBuffer(newblk, &buf);
    count = 0;
    // 进行排序
    sort(1, 16, &buf, 201, 301);
    sort(17, 48, &buf, 217, 317);
    printf("\n内排序结果：R:201-216  S:217-248\n");
    printf("\n归并排序结果（最终结果）：R:301-316  S:317-348\n\n");

    printf("------------------------------------------------------\n");
    printf("基于索引的关系选择算法 S.C = 128\n");
    printf("------------------------------------------------------\n");

    // 为S建立索引文件
    int indexBlkStart = 417; // 要写回的块的初始块号
    int startBlk = 317;      // 读取的初始块号
    int endBlk = 348;        // 读取的最终块号
    unsigned char *wbblk;
    int noRepeatNum = 0;     // 记录不重复的数
    int tempTuple[2];
    int countw = 0;          // 记录缓冲区写回块中的记录数
    // 为S建立索引文件
    while(startBlk<=endBlk){
        makeClear(&buf);
        // 读取块
        newblk = readBlockFromDisk(startBlk,&buf);
        wbblk = getNewBlockInBuffer(&buf);
        for(int i = 0; i < 7; i++){
            getTuple(newblk,i,tempTuple);
            // countw==7时，写回块已经满了，因此要写回磁盘
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
            // 判断该元组的id是否等于上个元组
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
    // 将未满的块写回到磁盘
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


    // 根据索引文件进行查找
    makeClear(&buf);
    int searchIndex = 128;          // 需要查找的id
    int indexStart = 417;           // 索引文件开始块
    int indexEnd = indexBlkStart;   // 索引文件结束块
    int needBlk = -1;               // 记录对应id在数据块的哪一个块中
    int breakFlag = 0;
    numio =0;
    while(indexStart<=indexEnd){
        // 读取块
        newblk = readBlockFromDisk(indexStart, &buf);
        numio++;
        printf("读入索引块%d\n",indexStart);
        // 遍历块，查询是否有对应id的索引记录
        for(int i = 0; i < 7; i++){
            getTuple(newblk, i, tuple);
            if(tuple[0]==searchIndex){
                needBlk = tuple[1];
                breakFlag = 1;
                break;
            }
            if(i ==6 ){
                printf("没有满足条件的元组\n");
            }
        }
        if(breakFlag){
            break;
        }
        indexStart++;
        freeBlockInBuffer(newblk, &buf);
    }

    // 根据得到的块号进行查找
    freeBlockInBuffer(newblk, &buf);
    int count1 = 0;                     // 记录是否找到了对应的id
    int count2 = 0;                     // 记录是否已经读完相应id
    wbblk = getNewBlockInBuffer(&buf);
    count = 0;                          // 记录wbblk中记录数
    int countnum = 0;                  // 记录索要查询的id的记录个数
    int writeBackBlk = 501;            // 查询结果写回的开始块
    int writeBack = writeBackBlk;
    int need = needBlk;
    // 根据得到的块号进行查找
    while(!count2){
        printf("读入数据块%d\n", needBlk);
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
    // 将未满的块写回到磁盘
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
    // 输出相关信息
    for(int i = writeBack; i <= writeBackBlk; i++){
        printf("\n注：结果写入磁盘：%d\n", i);
    }
    printf("\n满足选择条件的元组一共%d个\n", countnum);
    printf("\nIO读写一共%d次\n", numio);


    printf("------------------------------------------------------\n");
    printf("基于排序的连接算法\n");
    printf("------------------------------------------------------\n");
    // 遍历排序后的R、S块
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
        // 写回块满，将其写到磁盘
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
            printf("注：结果写入磁盘：%d\n",writeBack);
            wbblk = getNewBlockInBuffer(&buf);
            writeBack++;
        }
        // 如果R中的块中的元素全都已经读取完，读取下一个
        if(count1==7){
            rStartBlk++;
            count1 = 0;
            freeBlockInBuffer(rblk, &buf);
            freeBlockInBuffer(sblk, &buf);
            continue;
        }
        // 如果S中的块中的元素全都已经读完，读取下一个
        if(count2==7){
            sStartBlk++;
            count2 = 0;
            freeBlockInBuffer(rblk, &buf);
            freeBlockInBuffer(sblk, &buf);
            continue;
        }
        // 读取两个块中元素
        getTuple(rblk, count1, tuple1);
        getTuple(sblk, count2, tuple2);
        // 如果R块中元素的索引小于 S块中元素的索引，则取R块中下一个元素
        if(tuple1[0]<tuple2[0]){
            if(backFlag){
                backFlag = 0;
                sStartBlk = backblk;
                count2 = backIndex;
            }
            count1++;
        }
        // 如果R块中元素的索引大于 S块中元素的索引，则取S块中下一个元素
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
        // 如果R块中元素的索引等于 S块中元素的索引
        // 记录所处块号、索引， 将回溯标志记为1
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
    // 将未满的块写回到磁盘
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
        printf("注：结果写入磁盘：%d\n",writeBack);
    }
    printf("总共连接%d次\n", connectNum);
    makeClear(&buf);



    printf("------------------------------------------------------\n");
    printf("基于排序的集合的交算法\n");
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
        // 写回块满，将其写到磁盘
        if(countw==7){
            countw=0;
            for (int k = 0; k < 4; k++)
            {
                itoa(writeBack+1,str1,10);
                *(wbblk + 7*8 + k)= str1[k];
                *(wbblk + 7*8 + 4 + k)= '\0';
            }
            writeBlockToDisk(wbblk, writeBack, &buf);
            printf("注：结果写入磁盘：%d\n",writeBack);
            wbblk = getNewBlockInBuffer(&buf);
            writeBack++;
        }
        // 如果R中的块中的元素全都已经读取完，读取下一个
        if(count1==7){
            rStartBlk++;
            count1 = 0;
            freeBlockInBuffer(rblk, &buf);
            freeBlockInBuffer(sblk, &buf);
            continue;
        }
        // 如果S中的块中的元素全都已经读完，读取下一个
        if(count2==7){
            sStartBlk++;
            count2 = 0;
            freeBlockInBuffer(rblk, &buf);
            freeBlockInBuffer(sblk, &buf);
            continue;
        }
        // 读取两个块中元素
        getTuple(rblk, count1, tuple1);
        getTuple(sblk, count2, tuple2);
        // 如果R块中元素的索引小于 S块中元素的索引，则取R块中下一个元素
        if(tuple1[0]<tuple2[0]){
            count1++;
        }
        // 如果R块中元素的索引大于 S块中元素的索引，则取S块中下一个元素
        else if(tuple1[0]>tuple2[0]){
            count2++;
        }
        // 如果R块中元素的索引等于 S块中元素的索引
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
    // 将未满的块写回到磁盘
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
        printf("注：结果写入磁盘：%d\n",writeBack);
    }
    printf("R和S的交集有%d个元组\n", countnum);
    makeClear(&buf);
    return 0;
}

