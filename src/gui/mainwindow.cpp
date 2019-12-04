#include "mainwindow.hpp"
#include "ui_mainwindow.h"
#include <qdebug.h>
#include <fstream>
#include <std_msgs/Int8.h>
#include "../ar_core/ControlEvents.h"
#include <iomanip>
#include <sstream>
#include <iostream>

MainWindow::MainWindow(QWidget *parent) :
        QMainWindow(parent),
        ui(new Ui::MainWindow),
        ros_obj(this, ros::this_node::getName())
{
    ui->setupUi(this);

    // Set up ROS    // Create the image widget
    //imageWidget = new CVImageWidget();

    //  ui->imageWidgetUI
    //ui->imageLayout->addWidget(imageWidget);

    connect(ui->button_task_1, SIGNAL(released()), this, SLOT(task_1_clicked()) );
    connect(ui->button_task_2, SIGNAL(released()), this, SLOT(task_2_clicked()) );
    connect(ui->button_task_3, SIGNAL(released()), this, SLOT(task_3_clicked()) );
    connect(ui->button_task_4, SIGNAL(released()), this, SLOT(task_4_clicked()) );
    connect(ui->button_task_5, SIGNAL(released()), this, SLOT(task_5_clicked()) );
    connect(ui->button_task_6, SIGNAL(released()), this, SLOT(task_6_clicked()) );
    connect(ui->button_task_7, SIGNAL(released()), this, SLOT(task_7_clicked()) );
    connect(ui->button_task_8, SIGNAL(released()), this, SLOT(task_8_clicked()) );

    connect(ui->button_home_masters, SIGNAL(released()),
            this, SLOT(home_masters_clicked()) );

    connect(ui->checkBox_pub_imgs, SIGNAL(toggled(bool)),
            this, SLOT(pub_imgs_state_changed(bool)) );

    connect(ui->button_haptics_disable, SIGNAL(clicked()),
            this, SLOT(button_haptics_disable_checked()) );

    connect(ui->button_haptics_manual, SIGNAL(clicked()),
            this, SLOT(button_haptics_manual_checked()) );

    connect(ui->button_haptics_skill, SIGNAL(clicked()),
            this, SLOT(button_haptics_skill_checked()) );

    connect(ui->button_calib_arm1, SIGNAL(released()),
            this, SLOT(calib_arm1_clicked()) );

    connect(ui->button_calib_arm2, SIGNAL(released()),
            this, SLOT(calib_arm2_clicked()) );

    connect(ui->button_exit, SIGNAL(released()),
            this, SLOT(exit_clicked()) );

    connect(ui->button_kill_core, SIGNAL(released()),
            this, SLOT(kill_core_clicked()) );

    connect(ui->pause_button, SIGNAL(released()),
            this, SLOT(pause_clicked()) );

    ui->input_init_perf_1->setText("0.0");
    ui->input_init_perf_2->setText("0.0");
    ui->input_session->setText("3");
    //ui->input_num_trials->setText("3");
    ui->stop->setEnabled(false);
    ui->pause_button->setEnabled(false);

    ui->button_haptics_disable->setChecked(true);

    timer = new QTimer(this);

    connect(timer, SIGNAL(timeout()), this, SLOT(onTimeout()));

    timer->start(100);

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onTimeout()
{
    ui->line_edit_num_repetitions->setText(QString::number(ros_obj.GetRepetitionNumber()));

    ui->line_edit_duration->setText(QString::number(ros_obj.task_state.time_stamp, 'f', 1));

    std::vector<double> perf_hist;
    ros_obj.GetPerformanceHistory(perf_hist);
    std::stringstream perf_hist_str;
    for (int i = 0; i < perf_hist.size(); ++i) {
        perf_hist_str << std::fixed << std::setprecision(2)
                      <<perf_hist[i] <<" - ";
    }
    ui->text_perf_history->setText(perf_hist_str.str().c_str());


}

void MainWindow::showImage(){

    // Load an image
    //  cv::Mat image = cv::imread("/home/nearlab/Downloads/VRFT_R2014a/1dof.jpg", true);
    //  imageWidget->showImage(image);

}

void MainWindow::task_1_clicked(){
    ros_obj.SetStateLabel(1);
    ROS_DEBUG("clicked task 1");
    std_msgs::Int8 msg;
    msg.data =  CE_START_TASK1;
    ros_obj.publisher_control_events.publish(msg);
}

void MainWindow::task_2_clicked(){
    ros_obj.SetStateLabel(2);
    ROS_DEBUG("clicked task 2");
    std_msgs::Int8 msg;
    msg.data =  CE_START_TASK2;
    ros_obj.publisher_control_events.publish(msg);
}

void MainWindow::task_3_clicked(){
    ros_obj.SetStateLabel(3);
    ROS_DEBUG("clicked task 3");
    std_msgs::Int8 msg;
    msg.data =  CE_START_TASK3;
    ros_obj.publisher_control_events.publish(msg);
}


void MainWindow::task_4_clicked(){
    ros_obj.SetStateLabel(4);
    ROS_DEBUG("clicked task 4");
    std_msgs::Int8 msg;
    msg.data =  CE_START_TASK4;
    ros_obj.publisher_control_events.publish(msg);
}

void MainWindow::task_5_clicked()
{
    ros_obj.SetStateLabel(5);
    ROS_DEBUG("clicked task 5");
    std_msgs::Int8 msg;
    msg.data =  CE_START_TASK5;
    ros_obj.publisher_control_events.publish(msg);
    ros_obj.publisher_control_events.publish(msg);

}


void MainWindow::task_6_clicked()
{
    ros_obj.SetStateLabel(6);
    ROS_DEBUG("clicked task 6");
    std_msgs::Int8 msg;
    msg.data =  CE_START_TASK6;
    ros_obj.publisher_control_events.publish(msg);

}


void MainWindow::task_7_clicked()
{
    ros_obj.SetStateLabel(7);
    ROS_DEBUG("clicked task 7");
    std_msgs::Int8 msg;
    msg.data =  CE_START_TASK7;
    ros_obj.publisher_control_events.publish(msg);

}


void MainWindow::task_8_clicked()
{
    ros_obj.SetStateLabel(8);
    ROS_DEBUG("clicked task 8");
    std_msgs::Int8 msg;
    msg.data =  CE_START_TASK8;
    ros_obj.publisher_control_events.publish(msg);

}

//NULL FIELD
void MainWindow::button_haptics_disable_checked() {
    if(ui->button_haptics_disable->isChecked()){
        ros_obj.SetHapticsMode(1);
        double act = 0.0;//MIN(ui->input_ac_activation->text().toDouble(), 0
        // .0);
        ros_obj.OverrideACActivation(act);

    // ------- OLD DISABLE CODE ---------//
      /*if(ui->button_haptics_disable->isChecked()){
        ros_obj.SetHapticsMode(0);*/

    }

};


// CONVERGENT FIELD
void MainWindow::button_haptics_manual_checked() {
    if(ui->button_haptics_manual->isChecked()){
        ros_obj.SetHapticsMode(1);
        double act = 1.0;//MIN(ui->input_ac_activation->text().toDouble(), 1.0);
        ros_obj.OverrideACActivation(act);
    }

};

// DIVERGENT FIELD
void MainWindow::button_haptics_skill_checked() {
    if(ui->button_haptics_skill->isChecked()){
        ros_obj.SetHapticsMode(1);
        double act = -0.45;//MIN(ui->input_ac_activation->text().toDouble(), 1
        // .0);
        ros_obj.OverrideACActivation(act);
    }

};

// ------- OLD SKILL CODE --------//
/*void MainWindow::button_haptics_skill_checked() {
    if(ui->button_haptics_skill->isChecked())
        ros_obj.SetHapticsMode(2);
};*/



void MainWindow::home_masters_clicked(){

    std_msgs::Int8 msg;
    msg.data =  CE_HOME_MASTERS;
    ros_obj.publisher_control_events.publish(msg);

}

void MainWindow::pub_imgs_state_changed(bool state){

    qDebug() << "Toggled pub imgs";

    std_msgs::Int8 msg;
    if(state)
        msg.data =  CE_PUBLISH_IMGS_ON;
    else
        msg.data =  CE_PUBLISH_IMGS_OFF;

    ros_obj.publisher_control_events.publish(msg);
}

void MainWindow::calib_arm1_clicked(){
    std_msgs::Int8 msg;
    msg.data =  CE_CALIB_ARM1;
    ros_obj.publisher_control_events.publish(msg);

}

void MainWindow::calib_arm2_clicked(){
    std_msgs::Int8 msg;
    msg.data =  CE_CALIB_ARM2;
    ros_obj.publisher_control_events.publish(msg);

}


void MainWindow::kill_core_clicked(){

    ROS_INFO("Killiitang ar_core node...");

    std_msgs::Int8 msg;
    msg.data =  CE_EXIT;
    ros_obj.publisher_control_events.publish(msg);

}
void MainWindow::exit_clicked(){

    ROS_INFO("Exiting...");

    std_msgs::Int8 msg;
    msg.data =  CE_EXIT;
    ros_obj.publisher_control_events.publish(msg);

    ros_obj.CleanUpAndQuit();

    timer->stop();
    close();

    ROS_INFO("a...");


}


void MainWindow::on_record_clicked()
{
    std::string file_name_entered =
            "/home/charm/ATAR_Record/" +
            ui->file_name->text().toStdString() + "SubjectID" + ui->subject_id->text().toStdString() + "_" + ui->datelabel->text().toStdString() + '_' + ui->timelabel->text().toStdString();


    if(file_name_entered.empty()){
        ui->record->setChecked(false);
    }
    else{
        std::stringstream file_name;
        file_name<< file_name_entered << std::string(".csv");
        ros_obj.OpenRecordingFile(file_name.str());

        ui->file_name->setDisabled(true);
        ui->record->setText("Recording");
        ui->stop->setEnabled(true);
        ui->pause_button->setEnabled(true);

        // start recording
        int session = ui->input_session->text().toInt();
        double initial_performance_last= ui->input_init_perf_1->text().toDouble();
        double initial_performance_last2= ui->input_init_perf_2->text().toDouble();
        ros_obj.StartRecording(session, initial_performance_last,
                               initial_performance_last2);

        ui->input_init_perf_1->setDisabled(true);
        ui->input_init_perf_2->setDisabled(true);
        ui->input_session->setDisabled(true);
        ui->record->setEnabled(false);

        qDebug() << "Started recording." ;
    }
}


void MainWindow::pause_clicked()
{
    if(ui->record->isChecked()){
        if(ros_obj.IsRecording()) {
            ros_obj.PauseRecording();
            ui->pause_button->setText("Continue");
            ui->record->setEnabled(false);
            qDebug() << "Paused recording." ;
        }
        else{
            ros_obj.ContinueRecording();
            ui->pause_button->setText("Pause");
            ui->record->setEnabled(true);
            qDebug() << "Continued recording." ;
        }
    }
    else
        ui->pause_button->setChecked(false);

}


void MainWindow::on_stop_released()
{
    StopRecording();

}

void MainWindow::on_button_repeat_clicked()
{
    ros_obj.ResetCurrentAcquisition();
    qDebug() << "Resetting current acquisition" ;

    std_msgs::Int8 msg;
    msg.data =  CE_RESET_ACQUISITION;
    ros_obj.publisher_control_events.publish(msg);

}

void MainWindow::on_button_reset_clicked()
{
    StopRecording();
    ros_obj.ResetTask();
    std_msgs::Int8 msg;
    msg.data =  CE_RESET_TASK;
    ros_obj.publisher_control_events.publish(msg);

}

void MainWindow::StopRecording(){

    qDebug() << "Stopping the recording." ;

    if(ros_obj.IsRecording()){
        // stop writing to file and close it
        ros_obj.PauseRecording();
        ros_obj.CloseRecordingFile();

        // reset the interface
        ui->record->setEnabled(true);
        ui->record->setChecked(false);
        ui->record->setText("Record");

        ui->pause_button->setText("Pause");
        ui->pause_button->setChecked(false);

        ui->stop->setEnabled(false);

        ui->file_name->setDisabled(false);
        ui->input_init_perf_1->setDisabled(false);
        ui->input_init_perf_2->setDisabled(false);
        ui->input_session->setDisabled(false);
        ui->file_name->setText("");

        // clear the saved acquisition data
        ros_obj.ResetCurrentAcquisition();
    }

};
