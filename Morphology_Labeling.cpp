KImageGray KImageGray::BinaryDilate(const int& nType, const int nSize){
    KImageGray  igOut = *this;

    int d = nSize / 2;
    if(nType == _WHITE)
    {
        for(int i=d,ii=Row()-(d+1); ii>=d; i++,ii--)
            for(int j=d,jj=Col()-(d+1); jj>=d; j++,jj--)
            {
                try{
                    if(_ppA[i][j] == 0)
                        for(int p=-d;p<=d;p++)
                            for(int q=-d;q<=d;q++)
                                if(_ppA[i+p][j+q]){
                                    igOut._ppA[i][j] = 255;
                                    throw 1;
                                }
                }
                catch(int res){
                    continue;
                }
            }
    }
    else if(nType == _BLACK)
    {
        for(int i=d,ii=Row()-(d+1); ii>=d; i++,ii--)
            for(int j=d,jj=Col()-(d+1); jj>=d; j++,jj--)
            {
                try{
                    if(_ppA[i][j])
                        for(int p=-d;p<=d;p++)
                            for(int q=-d;q<=d;q++)
                                if(_ppA[i+p][j+q]==0){
                                    igOut._ppA[i][j] = 0;
                                    throw 1;
                                }
                }
                catch(int res){
                    continue;
                }
            }
    }

    return igOut;
}

KImageGray KImageGray::BinaryErode(const int& nType, const int nSize){
    KImageGray  igOut = *this;

    int d = nSize / 2;
    if(nType == _WHITE)
    {
        for(int i=d,ii=Row()-(d+1); ii>=d; i++,ii--)
            for(int j=d,jj=Col()-(d+1); jj>=d; j++,jj--)
            {
                try{
                    if(_ppA[i][j])
                        for(int p=-d;p<=d;p++)
                            for(int q=-d;q<=d;q++)
                                if(_ppA[i+p][j+q]==0){
                                    igOut._ppA[i][j] = 0;
                                    throw 1;
                                }
                }
                catch(int res){
                    continue;
                }
            }
    }
    else if(nType == _BLACK)
    {
        for(int i=d,ii=Row()-(d+1); ii>=d; i++,ii--)
            for(int j=d,jj=Col()-(d+1); jj>=d; j++,jj--)
            {
                try{
                    if(_ppA[i][j]==0)
                        for(int p=-d;p<=d;p++)
                            for(int q=-d;q<=d;q++)
                                if(_ppA[i+p][j+q]){
                                    igOut._ppA[i][j] = 255;
                                    throw 1;
                                }
                }
                catch(int res){
                    continue;
                }
            }
    }

    return igOut;
}

KArray<unsigned short> KImageGray::Neighbor_4(int* blobCnt, int* maxId){
    using KImageWord = KArray<unsigned short>;
    using KBlob = std::vector<KPOINT>;
    using KBlobList = std::vector<KBlob*>;

    int nRow = Row();
    int nCol = Col();

    KBlobList lBlobList;

    KImageWord iwColor(nRow, nCol);
    unsigned char uCur;
    unsigned char uLeft, uUp;
    unsigned short wLeftColor, wUpColor;
    int BlobCnt = 0;

    for(int i=0;i<nRow;i++){
        for(int j=0;j<nCol;j++){
            uCur = _ppA[i][j];

            if(uCur==0) continue;

            uLeft = j==0?0:_ppA[i][j-1];
            uUp = i==0?0:_ppA[i-1][j];
            wLeftColor = j==0?0:iwColor[i][j-1];
            wUpColor = i==0?0:iwColor[i-1][j];

            //if new blob is obtained
            if(uUp==0 && uLeft==0){
                //blob creation
                KBlob* opBlob = new KBlob;
                lBlobList.push_back(opBlob);
                //store the pixel coord.
                opBlob->push_back(KPOINT(i,j));
                //write the blob color
                iwColor[i][j] = lBlobList.size();

                BlobCnt++;
            }
            else if(uUp && uLeft==0){
                iwColor[i][j] = wUpColor;
                lBlobList[wUpColor-1]->push_back(KPOINT(i,j));
            }
            else if(uUp==0 && uLeft){
                iwColor[i][j] = wLeftColor;
                lBlobList[wLeftColor-1]->push_back(KPOINT(i,j));
            }
            else{
                if(wLeftColor == wUpColor){
                    iwColor[i][j] = wUpColor;
                    lBlobList[wUpColor-1]->push_back(KPOINT(i,j));
                }
                else{
                    for(auto& kp:*(lBlobList[wLeftColor-1])){
                        iwColor[kp.nX][kp.nY] = wUpColor;
                        lBlobList[wUpColor-1]->push_back(KPOINT(kp.nX, kp.nY));
                    }
                    iwColor[i][j] = wUpColor;
                    lBlobList[wUpColor-1]->push_back(KPOINT(i,j));
                    BlobCnt--;
                }
            }
        }
    }

    if(blobCnt != nullptr) *blobCnt = BlobCnt;
    if(maxId != nullptr) *maxId = lBlobList.size();

    for(auto&x:lBlobList)
        delete x;

    return iwColor;
}

KArray<unsigned short> KImageGray::Neighbor_8(int* blobCnt, int* maxId){
    using KImageWord = KArray<unsigned short>;
    using KBlob = std::vector<KPOINT>;
    using KBlobList = std::vector<KBlob*>;

    int nRow = Row();
    int nCol = Col();

    KBlobList lBlobList;

    KImageWord iwColor(nRow, nCol);
    unsigned char uCur;
    unsigned char uLeft, uUp, uDiag;
    unsigned short wLeftColor, wUpColor, wDiagColor;
    int BlobCnt = 0;

    for(int i=0;i<nRow;i++){
        for(int j=0;j<nCol;j++){
            uCur = _ppA[i][j];

            if(uCur==0) continue;

            uLeft = j==0?0:_ppA[i][j-1];
            uUp = i==0?0:_ppA[i-1][j];
            uDiag = (i==0||j==0)?0:_ppA[i-1][j-1];
            wLeftColor = j==0?0:iwColor[i][j-1];
            wUpColor = i==0?0:iwColor[i-1][j];
            wDiagColor = (i==0||j==0)?0:iwColor[i-1][j-1];

            //if new blob is obtained
            if(uUp==0 && uLeft==0 && uDiag==0){
                //blob creation
                KBlob* opBlob = new KBlob;
                lBlobList.push_back(opBlob);
                //store the pixel coord.
                opBlob->push_back(KPOINT(i,j));
                //write the blob color
                iwColor[i][j] = lBlobList.size();

                BlobCnt++;
            }
            else if(uUp && uLeft==0 && uDiag==0){
                iwColor[i][j] = wUpColor;
                lBlobList[wUpColor-1]->push_back(KPOINT(i,j));
            }
            else if(uUp==0 && uLeft && uDiag==0){
                iwColor[i][j] = wLeftColor;
                lBlobList[wLeftColor-1]->push_back(KPOINT(i,j));
            }
            else if(uUp==0 && uLeft==0 && uDiag){
                iwColor[i][j] = wDiagColor;
                lBlobList[wDiagColor-1]->push_back(KPOINT(i,j));
            }
            else if(uLeft && uUp && uDiag==0){
                if(wLeftColor==wUpColor){
                    iwColor[i][j] = wUpColor;
                    lBlobList[wUpColor-1]->push_back(KPOINT(i,j));
                }
                else{
                    //merge left to up
                    for(auto& kp:*(lBlobList[wLeftColor-1])){
                        iwColor[kp.nX][kp.nY] = wUpColor;
                        lBlobList[wUpColor-1]->push_back(KPOINT(kp.nX, kp.nY));
                    }
                    iwColor[i][j] = wUpColor;
                    lBlobList[wUpColor-1]->push_back(KPOINT(i,j));
                    BlobCnt--;
                }
            }
            else if(uLeft && uDiag && uUp==0){
                iwColor[i][j] = wLeftColor;
                lBlobList[wLeftColor-1]->push_back(KPOINT(i,j));
            }
            else if(uUp && uDiag && uLeft==0){
                iwColor[i][j] = wUpColor;
                lBlobList[wUpColor-1]->push_back(KPOINT(i,j));
            }
            else{ //uUp == uLeft == uDiag
                iwColor[i][j] = wUpColor;
                lBlobList[wUpColor-1]->push_back(KPOINT(i,j));
            }
        }
    }

    if(blobCnt != nullptr) *blobCnt = BlobCnt;
    if(maxId != nullptr) *maxId = lBlobList.size();

    for(auto&x:lBlobList)
        delete x;

    return iwColor;
}



void MainFrame::on_checkBox3by3_clicked()
{
    ui->checkBox5by5->setChecked(false);
    ui->checkBox3by3->setChecked(true);
}


void MainFrame::on_checkBox5by5_clicked()
{
    ui->checkBox3by3->setChecked(false);
    ui->checkBox5by5->setChecked(true);
}
//===========================================//

void MainFrame::on_pushDilationErosion_clicked()
{
    KImageGray igMain;
    if(_q_pFormFocused != 0 && _q_pFormFocused->ImageGray().Address() && _q_pFormFocused->ID() == "OPEN")
        igMain = _q_pFormFocused->ImageGray();
    else
        return;

    //Ostu Thresholding
    KImageGray igBin;
    KBINARIZATION_OUTPUT* opBinOutput = KHisto().Ostu(&igMain, &igBin);

    int nSize = (ui->checkBox3by3->isChecked()?3:5); //체크박스는 무조건 둘 중 하나는 되어있는 상태
    KImageGray dilatedImg = igBin.BinaryDilate(_WHITE, nSize); //BinaryDilate
    KImageGray erodedImg = igBin.BinaryErode(_WHITE, nSize);   //BinaryErode

    ImgForm<KImageGray>::Create(*this, "Dilated", dilatedImg);
    ImgForm<KImageGray>::Create(*this, "Eroded", erodedImg);
}


void MainFrame::on_checkBox4N_clicked()
{
    ui->checkBox8N->setChecked(false);
    ui->checkBox4N->setChecked(true);
}


void MainFrame::on_checkBox8N_clicked()
{
    ui->checkBox4N->setChecked(false);
    ui->checkBox8N->setChecked(true);
}


void MainFrame::on_pushLabel_clicked()
{
    KImageGray igMain;
    if(_q_pFormFocused != 0 && _q_pFormFocused->ImageGray().Address() && (_q_pFormFocused->ID() == "Dilated" || _q_pFormFocused->ID() == "Eroded"))
        igMain = _q_pFormFocused->ImageGray();
    else
        return;

    //Neighbor 실행
    int blobCnt, maxId;
    KArray<unsigned short> KWords;
    const char* ID;
    if(ui->checkBox4N->isChecked()){ //4-Neighbor가 체크되어 있다면
        KWords = igMain.Neighbor_4(&blobCnt, &maxId);
        ID = "4-Neighbor";
    }
    else{ //8-Neighbor가 체크되어 있다면
        KWords = igMain.Neighbor_8(&blobCnt, &maxId);
        ID = "8-Neighbor";
    }


    //Blob 별 색 부여
    KImageColor oImg(igMain.Row(), igMain.Col());
    vector<KCOLOR32> colors;
    colors.push_back(KCOLOR32(0,0,0));
    int dR = 97, dG = 83, dB = 67; //소수를 이용하여 색이 최대한 안 겹치도록 함

    for(int i = 1;i<=maxId;i++){ //Blob 라벨 별 RGB 생성
        int r = (i * dR) % 185 + 70;
        int g = (i * dG) % 185 + 70;
        int b = (i * dB) % 185 + 70;
        colors.push_back(KCOLOR32(r, g, b));
    }

    for(int i = KWords.Row() -1; i>=0;i--){ //Color Image 객체 생성
        for(int j = KWords.Col()-1;j>=0;j--){
            oImg._ppA[i][j].r = colors[KWords[i][j]].r;
            oImg._ppA[i][j].g = colors[KWords[i][j]].g;
            oImg._ppA[i][j].b = colors[KWords[i][j]].b;
        }
    }

    if(ui->listWidget->isVisible() == false)
        on_buttonShowList_clicked();
    ui->listWidget->addItem(QString("Number of labels ---> " + QString::number(blobCnt)));

    ImgForm<KImageColor>::Create(*this, ID, oImg);

}
