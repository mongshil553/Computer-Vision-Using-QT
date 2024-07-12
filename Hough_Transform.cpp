int KCircleHough::Init(void* pInfo)
{
	_oInfo = *(KCIRCLEHOUGH_INFO*)pInfo;

	//create voting table
    _lllVotes.resize(_oInfo.nNumGridCy);
    for(auto& itemY : _lllVotes)
    {
        itemY.resize(_oInfo.nNumGridCx);
        for(auto& itemX : itemY)
            itemX.resize(_oInfo.nNumGridR);
    }

	//equal ratio for quantization of center positon and radius
    _dSx = (double)(_oInfo.nRight - _oInfo.nLeft) / (double)(_oInfo.nNumGridCx-1); //Cx = nLeft  + k*_dSx
    _dSy = (double)(_oInfo.nBottom- _oInfo.nTop)  / (double)(_oInfo.nNumGridCy-1); //Cy = nTop   + k*_dSy
    _dSr = (double)(_oInfo.nMaxR  - _oInfo.nMinR) / (double)(_oInfo.nNumGridR -1); //R  = nMinR  + k*_dSr
    return 1;
}

void* KCircleHough::Run(void* pEdge, void* pFrom,void* pTo,void* p4, void* p5)
{
	int			nGridCx,nGridCy,nGridR;
	double		dRadius;
	double		dAngle;
	KPoint		ptCen;
	KEdge* 		opEdge = (KEdge*)pEdge;
	KCircle*	ccpFrom = (KCircle*)pFrom;
	KCircle*	ccpTo   = (KCircle*)pTo;

	//reset the voting table
    _lllVotes.resize(_oInfo.nNumGridCy);
    for(auto& itemY : _lllVotes)
    {
        itemY.resize(_oInfo.nNumGridCx);
        for(auto& itemX : itemY)
            itemX.resize(_oInfo.nNumGridR);
    }

	//Hough Transform
    for(auto& oEdgePxl : *opEdge)
    {
		//check if the edge is in effective region
        if(ccpTo && !ccpTo->InCircle((double)oEdgePxl.u,(double)oEdgePxl.v))
			continue;
        if(ccpFrom && ccpFrom->InCircle((double)oEdgePxl.u,(double)oEdgePxl.v))
			continue;

		//votes for radius
		nGridR 	= 1;
		dRadius = (double)_oInfo.nMinR;
		do{
			//compute circle centers for voting
            dAngle 	  =	_RADIAN(oEdgePxl.wAng);

			if(_oInfo.nDirEdge == _OUTWARD_EDGE)
			{
                ptCen._dX = oEdgePxl.u - dRadius*cos(dAngle);
                ptCen._dY = oEdgePxl.v - dRadius*sin(dAngle);
			}
            else //_INWARD_EDGE
			{
                ptCen._dX = oEdgePxl.u + dRadius*cos(dAngle);
                ptCen._dY = oEdgePxl.v + dRadius*sin(dAngle);
			}

			//compute grid coordinates
			nGridCx	  = (int)( (ptCen._dX - (double)_oInfo.nLeft)/_dSx );
			if(nGridCx < 2 || nGridCx > _oInfo.nNumGridCx-3)
				continue;
			nGridCy	  = (int)( (ptCen._dY - (double)_oInfo.nTop)/_dSy );
			if(nGridCy < 2 || nGridCy > _oInfo.nNumGridCy-3)
				continue;

			//votes to avoid edge noises
			for(int i=-2; i<3; i++)
				for(int j=-2; j<3; j++)
				{
                    _lllVotes[nGridCy+i][nGridCx+j][nGridR-1] += 0.7;
                    _lllVotes[nGridCy+i][nGridCx+j][nGridR]   += 0.7;
                    _lllVotes[nGridCy+i][nGridCx+j][nGridR+1] += 0.7;
				}
            _lllVotes[nGridCy][nGridCx][nGridR] += 0.3;

			//increment
			dRadius += _dSr;

		}while(++nGridR < _oInfo.nNumGridR-1);
	}

	//select peaks
	int	nGmax=0;
	for(int i=0; i<_oInfo.nNumGridCy; i++)
	{
		for(int j=0; j<_oInfo.nNumGridCx; j++)
		{
			for(int k=0; k<_oInfo.nNumGridR; k++)
                if(_lllVotes[i][j][k] > nGmax)
				{
                    nGmax 	= (int)(_lllVotes[i][j][k]);
					nGridCx	= j;
					nGridCy	= i;
					nGridR	= k;
				}
		}
	}

    int nThresh = (_oInfo.nThreshVote == 0 ? _PI*(_oInfo.nMinR + nGridR*_dSr)*0.5 : _oInfo.nThreshVote);
    if(nGmax < nThresh)
		return 0;


	//중심 및 반지름의 그리드 값으로 부터 실제 픽셀 단위로 환산하여 반환한다
    _oDetected.Create(_oInfo.nLeft+nGridCx*_dSx,_oInfo.nTop+nGridCy*_dSy,_oInfo.nMinR + nGridR*_dSr);
    return (void*)(&_oDetected);
}
//---------------------------------------------------------------------------

vector<KCircle*>& KCircleHough::FindAll(KEdge& opEdge){
    int			nGridCx,nGridCy,nGridR;
    double		dRadius;
    double		dAngle;
    KPoint		ptCen;

    //voting 초기화
    _lllVotes.resize(_oInfo.nNumGridCy);
    for(auto& itemY : _lllVotes)
    {
        itemY.resize(_oInfo.nNumGridCx);
        for(auto& itemX : itemY)
            itemX.resize(_oInfo.nNumGridR);
    }

    for(auto& pixel : opEdge){
        nGridR 	= 1;
        dRadius = (double)_oInfo.nMinR;
        do{
            //중심 방향 계산
            dAngle 	  =	_RADIAN(pixel.wAng);

            if(_oInfo.nDirEdge == _OUTWARD_EDGE) //바깥쪽
            {
                ptCen._dX = pixel.u - dRadius*cos(dAngle);
                ptCen._dY = pixel.v - dRadius*sin(dAngle);
            }
            else //안쪽
            {
                ptCen._dX = pixel.u + dRadius*cos(dAngle);
                ptCen._dY = pixel.v + dRadius*sin(dAngle);
            }

            nGridCx	  = (int)( (ptCen._dX - (double)_oInfo.nLeft)/ _dSx );
            if(nGridCx < 2 || nGridCx > _oInfo.nNumGridCx-3)
                continue;
            nGridCy	  = (int)( (ptCen._dY - (double)_oInfo.nTop)/ _dSy );
            if(nGridCy < 2 || nGridCy > _oInfo.nNumGridCy-3)
                continue;


            for(int i = -2;i<3;i++)
                for(int j = -2; j<3; j++)
                    for(int k = -1; k <2; k++)
                        _lllVotes[nGridCy+i][nGridCx+j][nGridR+k] += 0.7; //노이즈 허용
            _lllVotes[nGridCy][nGridCx][nGridR] += 0.3;

            dRadius += _dSr;
        }while(++nGridR < _oInfo.nNumGridR-1);
    }

    //voting 최대 찾기
    double	nGmax=0;
    for(int i=0; i<_oInfo.nNumGridCy; i++)
    {
        for(int j=0; j<_oInfo.nNumGridCx; j++)
        {
            for(int k=0; k<_oInfo.nNumGridR; k++)
                if(_lllVotes[i][j][k] > nGmax)
                {
                    nGmax 	= (int)(_lllVotes[i][j][k]);
                }
        }
    }

    vDetect.clear();
    if(nGmax < _oInfo.nThreshVote)
        return vDetect;

    vector<double> _lVote; //Threshold 기준으로 원 저장
    for(int i=1, ii=_oInfo.nNumGridCy-1; ii; i++,ii--)
    {
        for(int j=1, jj=_oInfo.nNumGridCx-1; jj; j++,jj--)
        {
            for(int k=1, kk=_oInfo.nNumGridR-1; kk; k++,kk--)
                if(_lllVotes[i][j][k] > _oInfo.nThreshVote) //주어진 Threshold보다 높을 때
                {
                    if(_lllVotes[i][j][k]/nGmax > 0.5) //최대 Voting의 절반 이상일 경우 <- 계산량 줄임
                    {
                        vDetect.push_back(new KCircle(_oInfo.nLeft+j*_dSx,_oInfo.nTop+i*_dSy,_oInfo.nMinR + k*_dSr));
                        _lVote.push_back(_lllVotes[i][j][k]);
                    }
                }
        }
    }

    //중복 제거
    int	   Big, Small;
    KCircle BigHalf;

    for(int i=0, ii=_lVote.size()-1; ii; i++, ii--)
        for(int j=i+1; j<_lVote.size(); j++){
            if(vDetect[i]->_dRadius > vDetect[j]->_dRadius){
                Big = i; Small = j;
            }
            else{
                Big = j; Small = i;
            }
            BigHalf = *vDetect[Big];
            BigHalf._dRadius *= 0.5;


            if(BigHalf.InCircle(vDetect[Small]->Center())){ //작은 원의 중심이 큰 원의 1/4크기의 내부에 있을 때, 같은 지점에서 원이 검출된 것이라 판단
                //Voting이 더 적은 원 제거
                if(_lVote[i] > _lVote[j]){
                    delete vDetect[j];

                    _lVote.erase(_lVote.begin() + j);
                    vDetect.erase(vDetect.begin() + j--);
                }
                else{
                    delete vDetect[i];

                    _lVote.erase(_lVote.begin() + i);
                    vDetect.erase(vDetect.begin() + i--);
                    break;
                }
            }
        }


    return vDetect;
}

int generalizedHough::Init(void* pInfo){
    _oInfo = *(KGENERALHOUGH_INFO*)pInfo;

    //create voting table
    _llllVotes.resize(_oInfo.nNumGridCy);
    for(auto& itemY : _llllVotes){
        itemY.resize(_oInfo.nNumGridCx);
        for(auto& itemX : itemY){
            itemX.resize(_oInfo.nNumGridAng);
            for(auto& itemAng : itemX){
                itemAng.resize(_oInfo.nNumGridScale);
            }
        }
    }

    _dSx  = (double)(_oInfo.nRight - _oInfo.nLeft) / (double)(_oInfo.nNumGridCx-1);  //Cx = nLeft  + k*_dSx
    _dSy  = (double)(_oInfo.nBottom- _oInfo.nTop)  / (double)(_oInfo.nNumGridCy-1);  //Cy = nTop   + k*_dSy
    _dSang = (double)(_oInfo.nMaxAng - _oInfo.nMinAng) / (double)(_oInfo.nNumGridAng - 1); //rotation = nMinAng + k*_dSang
    _dSscale  = (double)(_oInfo.nMaxS - _oInfo.nMinS)  / (double)(_oInfo.nNumGridScale - 1); //scale  = nMinS  + k*_dSs

    return 1;
}

void* generalizedHough::Run(void* pEdge, void*pFrom, void*pTo, void*p4, void* p5){
    int         nGridCx, nGridCy, nGridAng, nGridScale;
    double      dAngle;
    double      dScale;
    int wDir;
    double dTmp;
    KPoint      ptCen;
    KEdge*      opEdge = (KEdge*)pEdge;



    //voting 초기화
    _llllVotes.resize(_oInfo.nNumGridAng);
    for(auto& itemAng : _llllVotes){
        itemAng.resize(_oInfo.nNumGridScale);
        for(auto& itemS : itemAng){
            itemS.resize(_oInfo.nNumGridCy);
            for(auto& itemY : itemS){
                itemY.resize(_oInfo.nNumGridCx);
            }
        }
    }

    for(auto& oEdgePxl : *opEdge){
        nGridAng = 1;
        dAngle = (double)_oInfo.nMinAng;
        do{
            nGridScale = 1;
            dScale = (double)_oInfo.nMinS;

            do{
                dTmp = oEdgePxl.wAng - dAngle;
                wDir = ((((int)(dTmp/22.5)+1)>>1)&0x00000003);

                for(int k = 0, kk=_lpTables[wDir].size(); kk;k++,kk--){
                    ptCen._dX = oEdgePxl.u - _lpTables[wDir][k]->_dX * dScale * cos(_lpTables[wDir][k]->_dY + dAngle);
                    ptCen._dY = oEdgePxl.v - _lpTables[wDir][k]->_dX * dScale * sin(_lpTables[wDir][k]->_dY + dAngle);

                    //좌표 계산
                    nGridCx	  = (int)( (ptCen._dX - (double)_oInfo.nLeft)/_dSx );
                    if(nGridCx < 1 || nGridCx > _oInfo.nNumGridCx-2)
                        continue;
                    nGridCy	  = (int)( (ptCen._dY - (double)_oInfo.nTop)/_dSy );
                    if(nGridCy < 1 || nGridCy > _oInfo.nNumGridCy-2)
                        continue;

                    for(int l=-1; l<2; l++){
                        for(int m=-1; m<2; m++){
                            for(int n=-1; n<2;n++){
                                for(int o = -1; o<2;o++){
                                    _llllVotes[nGridAng+l][nGridScale+m][nGridCy+n][nGridCx+o] += 0.7; //노이즈 허용
                                }
                            }
                        }
                    }
                    _llllVotes[nGridAng][nGridScale][nGridCy][nGridCx] += 0.3;
                }

                dScale += _dSscale;
            }while(++nGridScale < _oInfo.nNumGridScale - 1);

            dAngle += _dSang;
        }while(++nGridAng < _oInfo.nNumGridAng - 1);
    }


    //최대 voting 선택
    double	nGmax=0;
    for(int i = 0; i<_oInfo.nNumGridAng; i++){
        for(int j=0; j<_oInfo.nNumGridScale; j++){
            for(int k=0; k<_oInfo.nNumGridCy; k++){
                for(int l=0;l<_oInfo.nNumGridCx; l++){
                        if(_llllVotes[i][j][k][l] > nGmax){
                            nGmax = (int)(_llllVotes[i][j][k][l]);
                            nGridCx = l;
                            nGridCy = k;
                            nGridScale = j;
                            nGridAng = i;
                        }
                }
            }
        }
    }

    int nThresh = _oInfo.nThreshVote;

    if(nGmax < nThresh)
        return 0;

    _oDetected.cX = _oInfo.nLeft + nGridCx * _dSx;
    _oDetected.cY = _oInfo.nTop + nGridCy * _dSy;
    _oDetected.rotation = _oInfo.nMinAng + nGridAng * _dSang;
    _oDetected.scale = _oInfo.nMinS + nGridScale * _dSscale;

    return (void*)(&_oDetected);
}

void generalizedHough::fit(QPolygon& poly){
    for(int i = 0; i<4; i++){
        int size = _lpTables[i].size();
        for(int j = 0; j<size; j++)
            delete _lpTables[i][j];
    }

    _lpTables[0].clear();
    _lpTables[1].clear();
    _lpTables[2].clear();
    _lpTables[3].clear();


    for (const auto& point : poly) {
        center += point;
    }
    center /= poly.size();


    double dTmp, dDist, dAng;
    int nDir;

    for(int j = 1, jj = poly.size()-2;jj;j++,jj--){
        dTmp = atan2(poly[j+1].y() - poly[j-1].y(), poly[j+1].x() - poly[j-1].x()) * 180 / M_PI; //0~360

        nDir = ((((int)(dTmp/22.5)+1)>>1) & 0x00000003);

        dDist = sqrt(_SQR(center.x()-poly[j].x()) + _SQR(center.y()-poly[j].y()));

        dAng = atan2(poly[j].y() - center.y(), poly[j].x() - center.x());

        _lpTables[nDir].push_back(new KPoint(dDist, dAng));
    }
}
//===========================================//

KMASK MASK::gauss(double sigma, int size){

    int _nHalf;

    if(size == 0)
    {
        //set convolution mask length
        _nHalf  	 = ( 3.0*sigma < 2.5 ? 2:(int)(3.0*sigma+0.5) );
        size = _nHalf*2 +1;
    }
    else{
        //set convolution mask length
        size   =  (size%2 == 0 ? size+1 : size);
        _nHalf =  (size-1) / 2;
    }

    KMASK _mask(size, size);

    //compute the mask
    int    i,j,ii,jj;
    double dTmp, dScale=0.0, dSigma2 = 2.0*_SQR(sigma);

    for(i=-_nHalf,ii=0; i<=_nHalf; i++,ii++)
    {
        dTmp = -i*exp(-(i*i)/2./sigma/sigma);

        for(j=-_nHalf,jj=0; j<=_nHalf; j++,jj++)
        {
            _mask[ii][jj] = dTmp*exp(-(j*j)/dSigma2);
            dScale += (i<0 ? _mask[ii][jj] : 0.0);
        }
    }
    for(i=0; i<size; i++)
        for(j=0; j<size; j++)
        {
            _mask[i][j] /= -dScale;
        }

    return _mask;
}

//===========================================//

void MainFrame::on_pushHoughCircle_clicked()
{
    KImageGray igImg = _q_pFormFocused->ImageGray();
    KCIRCLEHOUGH_INFO oInfo;

    oInfo.nLeft         = ui->lineMinR->text().toInt();
    oInfo.nTop          = ui->lineMinR->text().toInt();
    oInfo.nRight        = igImg.Col() - ui->lineMinR->text().toInt();
    oInfo.nBottom       = igImg.Row() - ui->lineMinR->text().toInt();
    oInfo.nMinR         = ui->lineMinR->text().toInt();
    oInfo.nMaxR         = ui->lineMaxR->text().toInt();
    oInfo.nNumGridCx    = (int)((oInfo.nRight - oInfo.nLeft + 1) / ui->lineResolution->text().toDouble());
    oInfo.nNumGridCy    = (int)((oInfo.nBottom - oInfo.nTop + 1) / ui->lineResolution->text().toDouble());
    oInfo.nNumGridR     = (int)((oInfo.nMaxR - oInfo.nMinR + 1) / ui->lineResolution->text().toDouble());
    oInfo.nDirEdge      = _OUTWARD_EDGE;
    oInfo.nThreshVote   = ui->lineThreshold->text().toInt();

    KEdge oEdge(ui->lineSigma->text().toDouble());
    KImageGray igEdge;
    oEdge.Canny(ui->lineLow->text().toInt(), ui->lineHigh->text().toInt(), igImg, igEdge);

    KPGM(igEdge).Save("./output/edge4Hough.pgm");

    KCircleHough oCircleHough;
    oCircleHough.Init(&oInfo);
    const vector<KCircle*>& opCircle = oCircleHough.FindAll(oEdge);

    if(opCircle.size() == 0){
        if(ui->listWidget->isVisible() == false)
            on_buttonShowList_clicked();
        ui->listWidget->insertItem(0, QString("There in no circle detected !"));

        return;
    }

    auto img = ImgForm<KImageColor>::Create(*this, "Circle Localization", igImg.GrayToRGB());

    for(auto& point : opCircle)
        img->DrawEllipse(QPoint((int)(point->_dCx), (int)(point->_dCy)), (int)(point->_dRadius), (int)(point->_dRadius), QColor(255,0,0), 2);
    img->update();
}


void MainFrame::on_pushHoughGeneral_clicked()
{
    FILE* fp = freopen("./data/mask.txt", "r", stdin);

    int N; cin >> N;

    QList<QPoint> mask_;
    for(int i = 0; i<N; i++){
        int x, y;
        cin >> x >> y;
        mask_.append(QPoint(x, y));
    }

    fclose(fp);

    KImageGray igImg = _q_pFormFocused->ImageGray();
    KGENERALHOUGH_INFO oInfo;
    QPolygon Mask(mask_);

    oInfo.nLeft = 0;
    oInfo.nTop = 0;
    oInfo.nRight = igImg.Col();
    oInfo.nBottom = igImg.Row();

    oInfo.nMinAng = ui->editHGMinAngle->text().toDouble();
    oInfo.nMaxAng = ui->editHGMaxAngle->text().toDouble();

    oInfo.nMinS = ui->editHGMinScale->text().toFloat();
    oInfo.nMaxS = ui->editHGMaxScale->text().toFloat();

    oInfo.nNumGridCx = (int)((oInfo.nRight - oInfo.nLeft + 1) / ui->editHGGridResolution->text().toDouble());
    oInfo.nNumGridCy = (int)((oInfo.nBottom - oInfo.nTop + 1) / ui->editHGGridResolution->text().toDouble());
    oInfo.nNumGridAng = (int)((double)((int)(((oInfo.nMaxAng - oInfo.nMinAng) / ui->editHGAngleResolution->text().toDouble()+0.00009)*pow(10,4))) / (double)pow(10,4)); //정밀도 보정
    oInfo.nNumGridScale = (int)((double)((int)(((oInfo.nMaxS - oInfo.nMinS) / ui->editHGScaleResolution->text().toDouble()+0.00009)*pow(10,4))) / (double)pow(10,4)); //정밀도 보정

    oInfo.nThreshVote   = ui->editHGThreshold->text().toInt();

    KEdge oEdge(ui->editHGSigma->text().toDouble());
    KImageGray igEdge;
    oEdge.Canny(ui->editHGLow->text().toInt(), ui->editHGHigh->text().toInt(), igImg, igEdge);

    KPGM(igEdge).Save("./output/edge4GeneralHough.pgm");

    generalizedHough oGeneralHough;
    oGeneralHough.Init(&oInfo);
    oGeneralHough.fit(Mask);

    KGENERALHOUGH_FOUND* found_info = (KGENERALHOUGH_FOUND*)oGeneralHough.Run((void*)&oEdge);
    if(found_info == 0){ //없다면
        if(ui->listWidget->isVisible() == false)
            on_buttonShowList_clicked();
        ui->listWidget->insertItem(0, QString("There is no mask detected !"));

        return;
    }

    auto getcenter = [&](QPolygon& q)->QPoint{ //중심 구하기
        QPoint center;
        for (const auto& point : q) {
            center += point;
        }
        center /= q.size();

        return center;
    };
    auto movecenter = [&](QPolygon& q, QPoint newCenter)->QPolygon{ //중심 이동하기

        QPolygon moved;

        QPoint oldCenter = getcenter(q);

        QPoint delta = newCenter - oldCenter;

        for (auto& point : q) {
            //point += delta;
            moved << QPoint(point + delta);
        }

        return moved;
    };
    auto resize = [&](QPolygon& q, double ratio)->QPolygon{ //크기 바꾸기
        QPolygon scaled;
        for(const auto& point : q){
            scaled << QPoint(point.x() * ratio, point.y() * ratio);
        }
        return scaled;
    };
    auto rotate = [&](QPolygon& q, double angle)->QPolygon{ //회전 시키기
        QPoint center = getcenter(q);

        QPolygon rotated;
        for (const auto& point : q) {
            double x = point.x() - center.x();
            double y = point.y() - center.y();
            double newX = x * cos(-angle * M_PI / 180) - y * sin(-angle * M_PI / 180);
            double newY = x * sin(-angle * M_PI / 180) + y * cos(-angle * M_PI / 180);
            rotated << QPoint(newX + center.x(), newY + center.y());
        }

        return rotated;
    };

    auto create = [&](QPolygon& src, KGENERALHOUGH_FOUND info)->QPolygon{ //중심 좌표, 크기, 회전 반영된 QPolygon 생성하기
        QPolygon scaled, resized, moved;
        scaled = resize(src, info.scale);
        resized = rotate(scaled, info.rotation);
        moved = movecenter(resized, QPoint(info.cX, info.cY));

        return moved;
    };

    QPolygon found_shape = create(Mask, *found_info);

    auto img = ImgForm<KImageColor>::Create(*this, "Mask Localization", igImg.GrayToRGB());
    img->DrawPolygon(found_shape, QColor(255, 0, 0), 2);
    img->update();


    if(ui->listWidget->isVisible() == false)
        on_buttonShowList_clicked();
    ui->listWidget->insertItem(0, QString("------------------------"));
    ui->listWidget->insertItem(0, QString(QString("Scale: ") + QString::number(found_info->scale)));
    ui->listWidget->insertItem(0, QString(QString("Angle: ") + QString::number(found_info->rotation)));
    ui->listWidget->insertItem(0, QString(QString("CenterY: ") + QString::number(found_info->cY)));
    ui->listWidget->insertItem(0, QString(QString("CenterX: ") + QString::number(found_info->cX)));
    ui->listWidget->insertItem(0, QString("------------------------"));
}
