template <class T> class ImgForm{
public:
    static void Create(MainFrame& _this, const char* ID, const T& Img){
        ImageForm* q_pForm = 0;
        for(auto form: _this._lImageForm){ //_lImageForm에서 ID랑 크기가 일치하는 Form 찾기
            if(form->ID() == ID && form->size() == _this._q_pFormFocused->size()){
                q_pForm = form;
                break;
            }
        }

        if(q_pForm) //일치하는 Form이 있다면
            q_pForm->Update(Img);
        else{ //일치하는 Form이 없다면
            q_pForm = new ImageForm(Img, ID, &_this);
            q_pForm->show();
            _this._lImageForm.push_back(q_pForm);
        }

        _this.UpdateUI(); //UI Update
    }
};

//===========================================//

KImageHSI KImageColor::RGBtoHSI() const{
    KImageHSI oImg;

    this->RGBtoHSI(oImg);

    return oImg;
}

KImageGray KImageHSI::get_h(){
    int nRow = Row();
    int nCol = Col();
    KImageGray oImg;
    oImg.Create(nRow, nCol);

    for(int i = 0; i<nRow; i++){
        for(int j = 0; j<nCol; j++){
            oImg._ppA[i][j] = (unsigned char)(_ppA[i][j].h * 255); //0~255로 변환
        }
    }

    return oImg;
}
KImageGray KImageHSI::get_s(){
    int nRow = Row();
    int nCol = Col();
    KImageGray oImg;
    oImg.Create(nRow, nCol);

    for(int i = 0; i<nRow; i++){
        for(int j = 0; j<nCol; j++){
            oImg._ppA[i][j] = (unsigned char)(_ppA[i][j].s * 255);
        }
    }

    return oImg;
}
KImageGray KImageHSI::get_i(){
    int nRow = Row();
    int nCol = Col();
    KImageGray oImg;
    oImg.Create(nRow, nCol);

    for(int i = 0; i<nRow; i++){
        for(int j = 0; j<nCol; j++){
            oImg._ppA[i][j] = (unsigned char)(_ppA[i][j].i * 255);
        }
    }

    return oImg;
}

void KImageColor::RGBtoNRG(KImageNRG& oImg) const{
    int nRow = Row();
    int nCol = Col();
    oImg.Create(nRow, nCol);

    int dSum;

    for(int i = 0; i<nRow; i++){
        for(int j = 0; j<nCol;j++){
            dSum = _ppA[i][j].r + _ppA[i][j].g + _ppA[i][j].b;
            if(dSum == 0){
                oImg._ppA[i][j].r = oImg._ppA[i][j].g = 0.0;
            }
            else{
                oImg._ppA[i][j].r = (float)_ppA[i][j].r / dSum;
                oImg._ppA[i][j].g = (float)_ppA[i][j].g / dSum;
            }
        }
    }
}
KImageNRG KImageColor::RGBtoNRG() const{
    KImageNRG oImg;

    this->RGBtoNRG(oImg);

    return oImg;
}

KImageGray KImageNRG::get_r(){
    int nRow = Row();
    int nCol = Col();
    KImageGray gImg;
    gImg.Create(nRow, nCol);

    for(int i = 0; i<nRow; i++){
        for(int j = 0; j<nCol; j++){
            gImg._ppA[i][j] = (unsigned char)(_ppA[i][j].r * 255);
        }
    }

    return gImg;
}
KImageGray KImageNRG::get_g(){
    int nRow = Row();
    int nCol = Col();
    KImageGray gImg;
    gImg.Create(nRow, nCol);

    for(int i = 0; i<nRow; i++){
        for(int j = 0; j<nCol; j++){
            gImg._ppA[i][j] = (unsigned char)(_ppA[i][j].g * 255);
        }
    }

    return gImg;
}

//===========================================//

void MainFrame::on_pushRGB2HSI_clicked()
{
    //포커스 된 ImageForm으로부터 영상을 가져옴
    KImageColor icMain;
    if(_q_pFormFocused != 0 && _q_pFormFocused->ImageColor().Address() && _q_pFormFocused->ID() == "OPEN")
        icMain = _q_pFormFocused->ImageColor();
    else
        return;

    //RGB to HSI
    KImageHSI hsiImg = icMain.RGBtoHSI();

    //Hue, Saturation, Intensity를 Gray Image로 출력
    ImgForm<KImageGray>::Create(*this, "Hue Image", hsiImg.get_h());
    ImgForm<KImageGray>::Create(*this, "Saturation Image", hsiImg.get_s());
    ImgForm<KImageGray>::Create(*this, "Intensity Image", hsiImg.get_i());
}

void MainFrame::on_pushRGB2NRG_clicked()
{
    //포커스 된 ImageForm으로부터 영상을 가져옴
    KImageColor icMain;
    if(_q_pFormFocused != 0 && _q_pFormFocused->ImageColor().Address() && _q_pFormFocused->ID() == "OPEN")
        icMain = _q_pFormFocused->ImageColor();
    else
        return;

    //RGB to NRG
    KImageNRG nrgImg = icMain.RGBtoNRG();

    //Normalized Red, Normalized Green을 Gray Image로 출력
    ImgForm<KImageGray>::Create(*this, "Normalized Red Image", nrgImg.get_r());
    ImgForm<KImageGray>::Create(*this, "Normalized Green Image", nrgImg.get_g());

}