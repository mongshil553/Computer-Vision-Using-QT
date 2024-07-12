void Histogram::Equalize(KImageColor& src, KImageColor& out){
    int nRow = src.Row();
    int nCol = src.Col();
    out.Create(nRow, nCol);
    Histogram normalized;
    normalized.collect(src).toCumulativeProb();

    for(int i = 0; i<nRow; i++){
        for(int j = 0; j<nCol; j++){
            out._ppA[i][j].r = (unsigned char)(normalized.chist.R[src._ppA[i][j].r] * 255 + 0.5);
            out._ppA[i][j].g = (unsigned char)(normalized.chist.G[src._ppA[i][j].g] * 255 + 0.5);
            out._ppA[i][j].b = (unsigned char)(normalized.chist.B[src._ppA[i][j].b] * 255 + 0.5);
        }
    }
}
Histogram& Histogram::Match(KImageColor& icTarget, KImageColor& icSource, KImageColor& out){
    Histogram targetHist, sourceHist;
    targetHist.collect(icTarget).toCumulativeProb();
    sourceHist.collect(icSource).toCumulativeProb();
    colorHist& t = targetHist.chist;
    colorHist& s = sourceHist.chist;

    auto argminfinder = [&](double t, std::vector<double>& s, unsigned char* map){
        double dMin = INFINITY, dDiff;

        for(int i=0; i<=255; i++)
            if((dDiff = _DIFF(t, s[i])) < dMin)
            {
                dMin = dDiff;

                if(map)
                    *map = i;
            }
    };

    unsigned char mapped_r[255] = {0,};
    unsigned char mapped_g[255] = {0,};
    unsigned char mapped_b[255] = {0,};

    for(int i = 0; i<=255; i++){
        argminfinder(t.R[i], s.R, &mapped_r[i]);
        argminfinder(t.G[i], s.G, &mapped_g[i]);
        argminfinder(t.B[i], s.B, &mapped_b[i]);
    }

    int nRow = icTarget.Row();
    int nCol = icTarget.Col();
    out.Create(nRow, nCol);
    for(int i = 0; i<nRow;i++){
        for(int j = 0; j<nCol; j++){
            out._ppA[i][j].r = mapped_r[icTarget._ppA[i][j].r];
            out._ppA[i][j].g = mapped_g[icTarget._ppA[i][j].g];
            out._ppA[i][j].b = mapped_b[icTarget._ppA[i][j].b];
        }
    }
    return *this;
}
Histogram& Histogram::collect(const KImageColor& Img){
    vector<double> r(256, 0);
    vector<double> g(256, 0);
    vector<double> b(256, 0);

    int nRow = Img.Row();
    int nCol = Img.Col();

    for(int i = 0; i<nRow; i++){
        for(int j = 0; j<nCol; j++){
            r[Img._ppA[i][j].r]++;
            g[Img._ppA[i][j].g]++;
            b[Img._ppA[i][j].b]++;
        }
    }
    atrb = "RGB";
    size = Img.Size();
    chist.R.swap(r); chist.G.swap(g); chist.B.swap(b);
    return *this;
}
Histogram& Histogram::collect(KImageGray& Img){
    ghist.I.reserve(256); std::fill(ghist.I.begin(), ghist.I.end(), 0);
    int nRow = Img.Row();
    int nCol = Img.Col();

    for(int i = 0; i<nRow; i++){
        for(int j = 0; j<nCol; j++){
            ghist.I[Img._ppA[i][j]]++;
        }
    }
    atrb = "GRAY";
    size = Img.Size();
    return *this;
}
Histogram& Histogram::toCumulativeProb(){
    if(this->atrb == "RGB"){
        for(int i = 1; i<=255; i++){
            chist.R[i] += chist.R[i-1];
            chist.G[i] += chist.G[i-1];
            chist.B[i] += chist.B[i-1];
        }
        for(int i = 0; i<=255; i++){
            chist.R[i] /= size;
            chist.G[i] /= size;
            chist.B[i] /= size;
        }
    }
    else if(this->atrb == "GRAY"){
        for(int i = 1; i<=255; i++){
            ghist.I[i] += ghist.I[i-1];
        }
        for(int i = 0; i<=255; i++){
            ghist.I[i] /= size;
        }
    }
    return *this;
}

//===========================================//

void MainFrame::on_pushHistogramEqualization_clicked()
{
    KImageColor icMain;
    if(_q_pFormFocused != 0 && _q_pFormFocused->ImageColor().Address())
        icMain = _q_pFormFocused->ImageColor();
    else
        return;

    Histogram Equalizer;
    KImageColor icEqualized;
    Equalizer.Equalize(icMain, icEqualized); //Histogram Equliaze
    ImgForm<KImageColor>::Create(*this, "Histogram Equalized", icEqualized);
}


void MainFrame::on_pushHistogramMatching_clicked()
{
    QFileDialog::Options q_Options = QFileDialog::DontResolveSymlinks |
                                     QFileDialog::DontUseNativeDialog;
    QString              q_stFile  = QFileDialog::getOpenFileName(this,
                                                    tr("Select a Target Image"), "./data",
                                                    "Image Files(*.ppm *.pgm *.tif)", 0, q_Options);

    ImageForm* source;
    if(q_stFile.length() == 0)
        return;
    source = ImgForm<QString>::Create(*this, "Target Image", q_stFile);
    if(source->Atrb() != "RGB"){ //Check if not Color Image
        this->CloseImageForm(source); //Close recently opened ImageForm
        if(ui->listWidget->isVisible() == false)
            on_buttonShowList_clicked();
        ui->listWidget->addItem(QString("Select only Image color."));
        return;
    }
    source->show();
    KImageColor icSource = source->ImageColor();

    q_stFile = QFileDialog::getOpenFileName(this, tr("Select a Source Image"), "./data",
                                            "Image Files(*.bmp *.ppm *.pgm *.tif)", 0, q_Options);

    if(q_stFile.length()==0){
        this->CloseImageForm(source); //Close source image
        return;
    }
    ImageForm* target;
    target = ImgForm<QString>::Create(*this, "Source Image", q_stFile);
    if(target->Atrb() != "RGB"){ //Check if not Color Image
        this->CloseImageForm(source);
        this->CloseImageForm(target); //Close recenty opened ImageForm
        if(ui->listWidget->isVisible() == false)
            on_buttonShowList_clicked();
        ui->listWidget->addItem(QString("Select only Image color."));
        return;
    }
    target->show();
    KImageColor icTarget = target->ImageColor();

    Histogram Matching;
    KImageColor icMatched;
    Matching.Match(icTarget, icSource, icMatched); //Histogram Matching
    ImgForm<KImageColor>::Create(*this, "Histogram Matched", icMatched);
}

