#include "Telemedicine.h"

Telemedicine::Telemedicine(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
    initMainWindow();
    //基本信息视图
    model = new QSqlTableModel(this);
    model->setTable("basic_inf");
    model->select();
    //附加信息视图
    model_d =new QSqlTableModel(this);
    model_d  ->setTable("datails_inf");
    model_d->select();
    //数据网络信息加载
    ui.basicTableView->setModel(model);
    //初始化表单患者信息
    onTableSelectChange(0);

}

Telemedicine::~Telemedicine()
{}
void Telemedicine::initMainWindow()
{
    QString ctImgPath = "CT.jpg";
    Mat ctImg = imread(ctImgPath.toLatin1().data());
    cvtColor(ctImg, ctImg, COLOR_BGR2RGB);
    myCtImg = ctImg;
    myCtQImage = QImage((const unsigned char*)(ctImg.data), ctImg.cols, ctImg.rows, QImage::Format_RGB888);
    ctImgShow();
    //时间日期更新
    QDate date = QDate::currentDate();
    int year = date.year();
    ui.yearLcdNumber->display(year);
    int month = date.month();
    ui.mounthLcdNumber->display(month);
    int day = date.day();
    ui.dayLcdNumber->display(day);
    myTimer = new QTimer();
    myTimer->setInterval(1000);
    myTimer->start();

    connect(myTimer, SIGNAL(timeout()), this, SLOT(onTimeOut()));
}
void Telemedicine::ctImgRead()
{
    QString ctImgName = QFileDialog::getOpenFileName(this, "载入CT相片", ".", "Image File(*.png *.jpg *.jpeg *.bmp)");
    if (ctImgName.isEmpty())return;
    Mat ctRgbImg, ctGrayImg;
    Mat ctImg = imread(ctImgName.toLatin1().data());
    cvtColor(ctImg, ctRgbImg, COLOR_BGR2RGB);
    cvtColor(ctRgbImg, ctGrayImg, COLOR_RGB2GRAY);
    myCtImg = ctRgbImg;
    myCtGrayImg = ctGrayImg;
    myCtQImage = QImage((const unsigned char*)(ctRgbImg.data), ctRgbImg.cols, ctRgbImg.rows, QImage::Format_RGB888);
    ctImgShow();
}
void Telemedicine::ctImgProc()
{
    

    // 初始化时建立安全连接
    connect(&time1, &QTimer::timeout, this, [this]() {
        QCoreApplication::processEvents();
        });

    connect(&time2, &QTimer::timeout, this, [this]() {
        switch (flag) {
        case 0:
            ctImgHoughCircles();
            ui.progressBar->setValue(40); // 直接设置精确进度
            flag++;
            break;
        case 1:
            ctImgShow();
            ui.progressBar->setValue(80);
            flag++;
            break;
        case 2:
            time1.stop();
            time2.stop();
            ui.progressBar->setValue(100);
            QMessageBox::information(this, tr("完毕"),
                tr("子宫内壁见椭球形阴影，疑似子宫肌瘤"));
            flag = 0;
            break;
        }
        });

    // 启动定时器
    time1.start(50);
    time2.start(2000);
    ui.progressBar->setValue(20);  // 设置合理初始值
   


}
void Telemedicine::ctImgSave()
{
    QFile image("E:\\QtProject\\Telemedicine\\Telemedicine");
    if (!image.open(QIODevice::ReadWrite))return;
    QByteArray qba;
    QBuffer buf(&qba);
    buf.open(QIODevice::WriteOnly);
    myCtQImage.save(&buf, "JPG");
    image.write(qba);

}
void Telemedicine::ctImgShow()
{
    ui.CT_Img_Label->setPixmap(QPixmap::fromImage(myCtQImage.scaled(ui.CT_Img_Label->size(), Qt::KeepAspectRatio)));
}
void Telemedicine::ctImgHoughCircles()
{
    Mat ctGrayImg = myCtGrayImg.clone();
    Mat ctColorImg;
    // 修改颜色转换常量
    cvtColor(ctGrayImg, ctColorImg, cv::COLOR_GRAY2BGR);
    GaussianBlur(ctGrayImg, ctGrayImg, Size(9, 9), 2, 2);
    vector<Vec3f> h_circles;
    // 修改霍夫变换常量
    HoughCircles(ctGrayImg, h_circles, cv::HOUGH_GRADIENT, 2, ctGrayImg.rows / 8, 200, 100);

    int processValue = 45;
    ui.progressBar->setValue(processValue);

   
    // 建议改用 QEventLoop 替代忙等待
    QEventLoop loop;
    QTimer::singleShot(2000, &loop, &QEventLoop::quit);
    loop.exec();

    for (size_t i = 0; i < h_circles.size(); i++)
    {
        Point center(cvRound(h_circles[i][0]), cvRound(h_circles[i][1]));
        int h_radius = cvRound(h_circles[i][2]);
        // 添加cv::命名空间明确范围
        cv::circle(ctColorImg, center, h_radius, Scalar(238, 0, 238), 3, 8, 0);
        cv::circle(ctColorImg, center, 3, Scalar(238, 0, 0), -1, 8, 0);

        // 更新进度条（避免超过100%）
        processValue = std::min(100, processValue + 1);
        ui.progressBar->setValue(processValue);
    }

    myCtImg = ctColorImg;
    // 确保图像连续且格式正确
    myCtQImage = QImage(myCtImg.data,
        myCtImg.cols,
        myCtImg.rows,
        QImage::Format_RGB888);

 
   
}
void Telemedicine::onTableSelectChange(int row)
{
    int r = 1;
    if (row != 0)r = ui.basicTableView->currentIndex().row();
    QModelIndex index;
    index = model->index(r, 1);
    ui.nameLabel->setText(model->data(index).toString());
    index = model->index(r, 2);
    QString sex = model->data(index).toString();
    (sex.compare("男") == 0) ? ui.maleRadioButton->setChecked(true) : ui.femaleRadioButton->setChecked(true);
    index = model->index(r, 4);
    QDate date;
    int now = date.currentDate().year();
    int bir = model->data(index).toDate().year();
    ui.ageSpinBox->setValue(now - bir);
    index = model->index(r, 3);
    QString ethnic = model->data(index).toString();
    ui.ethniComboBox->setCurrentText(ethnic);
    index = model->index(r, 0);
    QString ssn = model->data(index).toString();
    ui.ssnLineEdit->setText(ssn);
    showUserPhoto();


}
void Telemedicine::showUserPhoto()
{
    QPixmap photo;
    QModelIndex index;
    for (int i = 0; i < model_d->rowCount(); i++) {
        index = model_d->index(i, 0);
        QString current_name = model_d->data(index).toString();
        if (current_name.compare(ui.nameLabel->text()) == 0) {
            index = model_d->index(i, 2);
            break;
        }

    }
    photo.loadFromData(model_d->data(index).toByteArray(), "JPG");
    ui.photoLabel->setPixmap(photo);

}
void Telemedicine::on_basicTableView_clicked(const QModelIndex& index)
{
    onTableSelectChange(1);
}
void Telemedicine::on_tabWidget_tabBarClicked(int index)
{
    if (index == 1) {
        QModelIndex index;
        for (int i = 0; i < model_d->rowCount(); i++) {
            index = model_d->index(i, 0);
            QString current_name = model_d->data(index).toString();
            if (current_name.compare(ui.nameLabel->text()) == 0) {
                index = model_d->index(i, 1);
                break;
            }
        }
        ui.caseTextEdit->setText(model_d->data(index).toString());
        ui.caseTextEdit->setFont(QFont("楷体", 12));
    }
}
void Telemedicine::onTimeOut()
{
    QTime time = QTime::currentTime();
    ui.timeEdit->setTime(time);
}
void Telemedicine::on_startPushButton_clicked()
{
    ctImgRead();
    QTimer time;
    time.setSingleShot(true);
    time.start(5000);
    ui.progressBar->setMaximum(0);
    ui.progressBar->setMinimum(0);
    while (time.isActive()) {
        QCoreApplication::processEvents();
    }
    ui.progressBar->setMaximum(100);
    ui.progressBar->setMinimum(0);
    ctImgProc();
    
    ui.progressBar->setValue(0);
    ctImgSave();
    

}
