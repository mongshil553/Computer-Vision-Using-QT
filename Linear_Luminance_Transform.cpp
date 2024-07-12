KImageColor KImageColor::LuminanceContrastTransform(const unsigned char a, const unsigned char b, const unsigned char a_, const unsigned char b_){
    int nRow = Row();
    int nCol = Col();
    double coef = (double)(b_-a_)/(double)(b-a);
    unsigned char Xin;

    KImageColor oImg;
    oImg.Create(nRow, nCol);

    for(int i = 0; i<nRow;i++){
        for(int j = 0; j<nCol;j++){
            Xin = (_ppA[i][j].r<a)?a:(_ppA[i][j].r>b?b:_ppA[i][j].r);
            oImg._ppA[i][j].r = (unsigned char)(coef*(Xin - a) + (double)a_);
            Xin = (_ppA[i][j].g<a)?a:(_ppA[i][j].g>b?b:_ppA[i][j].g);
            oImg._ppA[i][j].g = (unsigned char)(coef*(Xin - a) + (double)a_);
            Xin = (_ppA[i][j].b<a)?a:(_ppA[i][j].b>b?b:_ppA[i][j].b);
            oImg._ppA[i][j].b = (unsigned char)(coef*(Xin - a) + (double)a_);
        }
    }

    return oImg;
}

//===========================================//

void MainFrame::on_pushLuminanceContrastTransform_clicked()
{
    KImageColor icMain;
    if(_q_pFormFocused != 0 && _q_pFormFocused->ImageColor().Address() && _q_pFormFocused->ID() == "OPEN")
        icMain = _q_pFormFocused->ImageColor();
    else
        return;

    const unsigned char aVal = ui->spinA->value();
    const unsigned char bVal = ui->spinB->value();
    const unsigned char a_Val = ui->spinA_->value();
    const unsigned char b_Val = ui->spinB_->value();


    if(aVal == bVal){
        if(ui->listWidget->isVisible() == false)
            on_buttonShowList_clicked();
        ui->listWidget->addItem(QString("Error: a and b cannot be same; division by zero"));
        return;
    }

    //Luminance Contrast Transform
    KImageColor cImg = icMain.LuminanceContrastTransform(aVal, bVal, a_Val, b_Val);

    ImgForm<KImageColor>::Create(*this, "Luminance Contrast Transform", cImg);
}

