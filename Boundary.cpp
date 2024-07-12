KMASK MASK::set(int nRow, int nCol, ...){
    KMASK _mask(nRow, nCol);

    va_list ap;
    va_start(ap, nCol);

    for(int i = 0; i<nRow; i++){
        for(int j = 0; j<nCol; j++){
           _mask. _ppA[i][j] = va_arg(ap, double);
        }
    }

    va_end(ap);

    return _mask;
}
KMASK MASK::fill(int nRow, int nCol, double num){
    KMASK _mask(nRow, nCol);
    for(int i = 0;i<nRow;i++)
        for(int j = 0; j<nCol;j++)
            _mask._ppA[i][j] = num;

    return _mask;
}
KMASK MASK::setv(int nRow, int nCol, std::vector<double>& val){
    KMASK _mask(nRow, nCol);

    auto it = val.begin();
    for(int i = 0; i<nRow; i++){
        for(int j = 0; j<nCol; j++){
            _mask. _ppA[i][j] = *it;
            it++;
        }
    }

    return _mask;
}

KImageGray KImageGray::BoundaryFG(const int nSize, const KMASK& mask){
    unsigned int nRow = Row();
    unsigned int nCol = Col();

    KImageGray igOut = this->BinaryErode(_WHITE, nSize, mask);

    for(unsigned int i=0;i<nRow;i++){
        for(unsigned int j=0;j<nCol;j++){
            if(this->_ppA[i][j]){
                if(igOut._ppA[i][j] == 0)
                    igOut._ppA[i][j] = 255;
                else
                    igOut._ppA[i][j] = 0;
            }
        }
    }

    return igOut;
}
KImageGray KImageGray::BoundaryBG(const int nSize, const KMASK& mask){
    unsigned int nRow = Row();
    unsigned int nCol = Col();

    KImageGray igOut = this->BinaryDilate(_WHITE, nSize, mask);

    for(unsigned int i=0;i<nRow;i++){
        for(unsigned int j=0;j<nCol;j++){
            if(igOut._ppA[i][j] ==0)
                continue;
            else if(this->_ppA[i][j])
                igOut[i][j] = 0;
        }
    }

    return igOut;
}

//===========================================//

void MainFrame::on_checkBoxOpen3by3_clicked()
{
    ui->checkBoxOpen5by5->setChecked(false);
    ui->checkBoxOpen3by3->setChecked(true);
}

void MainFrame::on_checkBoxOpen5by5_clicked()
{
    ui->checkBoxOpen3by3->setChecked(false);
    ui->checkBoxOpen5by5->setChecked(true);
}

void MainFrame::on_pushOpening_clicked()
{
    KImageGray igMain;
    if(_q_pFormFocused != 0 && _q_pFormFocused->ImageGray().Address() && _q_pFormFocused->ID() == "OPEN")
        igMain = _q_pFormFocused->ImageGray();
    else
        return;

    KImageGray igBin;
    KBINARIZATION_OUTPUT* opBinOutput = KHisto().Ostu(&igMain, &igBin);

    KImageGray igOpen;
    int nSize = (ui->checkBoxOpen3by3->isChecked())?3:5;
    igOpen = igBin.BinaryOpening(_WHITE, nSize, nSize);

    ImgForm<KImageGray>::Create(*this, "Opening", igOpen);
}


void MainFrame::on_checkBoxClose3by3_clicked()
{
    ui->checkBoxClose5by5->setChecked(false);
    ui->checkBoxClose3by3->setChecked(true);
}


void MainFrame::on_checkBoxClose5by5_clicked()
{
    ui->checkBoxClose3by3->setChecked(false);
    ui->checkBoxClose5by5->setChecked(true);
}


void MainFrame::on_pushClosing_clicked()
{
    KImageGray igMain;
    if(_q_pFormFocused != 0 && _q_pFormFocused->ImageGray().Address() && _q_pFormFocused->ID() == "OPEN")
        igMain = _q_pFormFocused->ImageGray();
    else
        return;

    KImageGray igBin;
    KBINARIZATION_OUTPUT* opBinOutput = KHisto().Ostu(&igMain, &igBin);

    int nSize = (ui->checkBoxClose3by3->isChecked())?3:5;
    KImageGray igClose = igBin.BinaryClosing(_WHITE, nSize, nSize);

    ImgForm<KImageGray>::Create(*this, "Closing", igClose);
}


void MainFrame::on_checkBoxBoundaryMb4_clicked()
{
    ui->checkBoxBoundaryMb8->setChecked(false);
    ui->checkBoxBoundaryMb4->setChecked(true);
}


void MainFrame::on_checkBoxBoundaryMb8_clicked()
{
    ui->checkBoxBoundaryMb4->setChecked(false);
    ui->checkBoxBoundaryMb8->setChecked(true);
}


void MainFrame::on_checkBoxBoundaryFG_clicked()
{
    ui->checkBoxBoundaryBG->setChecked(false);
    ui->checkBoxBoundaryFG->setChecked(true);
}


void MainFrame::on_checkBoxBoundaryBG_clicked()
{
    ui->checkBoxBoundaryFG->setChecked(false);
    ui->checkBoxBoundaryBG->setChecked(true);
}


void MainFrame::on_pushBoundary_clicked()
{
    KImageGray igMain;
    if(_q_pFormFocused != 0 && _q_pFormFocused->ImageGray().Address() && _q_pFormFocused->ID() == "OPEN")
        igMain = _q_pFormFocused->ImageGray();
    else
        return;

    KImageGray igBin;

    KBINARIZATION_OUTPUT* opBinOutput = KHisto().Ostu(&igMain, &igBin);

    KMASK _mask;
    if(ui->checkBoxBoundaryMb4->isChecked())
        _mask = MASK::set(3, 3, 0., 1., 0., 1., 1., 1., 0., 1., 0.);
    else
        _mask = MASK::fill(3, 3, 1.);

    KImageGray igBoundary;
    const char* ID;
    if(ui->checkBoxBoundaryFG->isChecked()){
        igBoundary = igBin.BoundaryFG(3, _mask); //3x3 mask
        ID = "BoundaryFG";
    }
    else{
        igBoundary = igBin.BoundaryBG(3, _mask); //3x3 mask
        ID = "BoundaryBG";
    }

    ImgForm<KImageGray>::Create(*this, ID, igBoundary);
}

