/*

template <typename T>
void add_slot(const std::string &slot_name, T *instance, void (T::*func)(const std::shared_ptr<info_packet> &))
        {
            uint32_t id = generate_slot_id();
            std::function<void(const std::shared_ptr<info_packet> &)> slot_function = [instance, func](const std::shared_ptr<info_packet> &packet)
            {   
                if(func)
                    (instance->*func)(packet);
            };
            slot new_slot(slot_function, id, slot_name);
            m_slots[slot_name] = std::move(new_slot);
            return;
        }

*/

#pragma once

#include <iostream>
#include <QObject>
#include <QThread>
#include <QProcess>
#include <QFile>
#include <QTextStream>

    


#include "communication/gp_publisher.h"
#include "communication/gp_packet.h"


class Worker : public QObject
{
    Q_OBJECT

public:
    template <typename T>
    void set_job(const std::string &job_name, T *instance, void (T::*func)())
    {
        m_function = [instance, func]()
                                            {   
                                                if(func)
                                                    (instance->*func)();
                                            };
        
        return;
    }
    
public slots:
    void doWork()
    {
        QString result;

    
        if (m_function)
        {
            m_function();
        }
        
        emit resultReady(result);
    }

    
 private :
        
        std::function<void()> m_function;
signals:
    void resultReady(const QString &result);
};


class ProcessController : public QObject
{
    Q_OBJECT
    QThread workerThread;

    QString swp = "0/1000";
    QString surf_folds = "-";

   

public:
    ProcessController()
    {
        
        process = new QProcess();
        QObject::connect(process, &QProcess::readyReadStandardOutput, this, &ProcessController::readData);
        QObject::connect(process, &QProcess::finished, this, &ProcessController::process_finished);

        
    }
    ~ProcessController()
    {
        workerThread.quit();
        workerThread.wait();
    }

    template <typename T>
    void setJob(const std::string &job_name, T *instance, void (T::*func)())
    {
        Worker *worker = new Worker;
        worker->set_job(job_name, instance, func);
        worker->moveToThread(&workerThread);
        connect(&workerThread, &QThread::finished, worker, &QObject::deleteLater);
        connect(this, &ProcessController::operate, worker, &Worker::doWork);
        connect(worker, &Worker::resultReady, this, &ProcessController::handleResults);
        workerThread.start();
    }

    template <typename T>
    void setResultHandler(const std::string &job_name, T *instance, void (T::*func)())
    {
         m_result_handler = [instance, func]()
                                            {   
                                                if(func)
                                                    (instance->*func)();
                                            };
        
        return;

    }


    void start()
    {
        std::cout << "Work initiation\n";

        workerThread.start();
        emit operate("ads");
    }

    QProcess *process;

    void process_finished(int exitCode){
        gridpro_comms::post_packet("mainwindow", "stop_progress", {{"", ""}});
        std::map<QString, QString> data;
        data["heading"] = "GGRID Execution Completed.";
        data["message"] = QString("Sweeps : " + swp + "/1000<br> No of Surface Folds: " + surf_folds);
        gridpro_comms::post_packet("mainwindow", "show_information", data);
    }

    void readData()
    {
        QString output = process -> readAllStandardOutput();
        output = output.trimmed();
        //std::cout<<"{IGNORE} "<<output.toStdString()<<std::endl;

      

        bool update = false;
        if(output.contains("swp"))
        {
            const int pos = output.lastIndexOf("swp");
            output = output.mid(pos).trimmed();
            swp = output.split(" ")[1];

            std::cout<<"[SWEEP]:"<<swp.toStdString()<<std::endl;
            //if((swp.toInt()+1)%100 == 0)
                update = true;
        }
        else if(output.startsWith("surf"))
        {
            
            auto foldslist = output.split("\n")[0].split(" ");
            surf_folds = foldslist[foldslist.size() - 2];
            std::cout<<"[SURF FOLDS]:"<<surf_folds.toStdString()<<std::endl;
            update = true;
        }
        /*else if(output.startsWith("fold count"))
        {
            surf_folds = output.split(":")[1].trimmed().split("\n")[0];
            std::cout<<"[FOLDS]:"<<surf_folds.toStdString()<<std::endl;
            //update = true;
        }*/

        if(update)
        {
            std::map<QString, QString> data;
            data["heading"] = "GGRID";
            data["message"] = QString("Sweeps : " + swp + "/1000<br> No of Surface Folds: " + surf_folds);
            
            gridpro_comms::post_packet("mainwindow", "show_information", data);
        }
        if(output.startsWith("End Grid Session"))
        {
            std::map<QString, QString> data;
            data["heading"] = "Finishing GGRID";
            data["message"] = QString("Sweeps : " + swp + "/1000<br> No of Surface Folds: " + surf_folds);
            
            gridpro_comms::post_packet("mainwindow", "show_information", data);
        }

    }

public slots:
    void handleResults(const QString &)
    {
        std::cout << "Work done\n";
        if(m_result_handler)
            m_result_handler();


    }

    
private:
    std::function<void()>  m_result_handler;


signals:
    void operate(const QString &);
};


class GGrid_Process : public QObject
{
    Q_OBJECT
    QThread workerThread;

    QString swp = "0";
    QString surf_folds = "-";
    bool success;
    bool terminated;
    int number_of_sweeps      = 1000;
    int output_frequency      = 100;
    double global_density         = 1.0;
    QString schedule = "#This is generated from  Gridpro Vertical.\n"
                        "%1step %2: -c all 1.0 0 -C all 1.0 3 -r -S %3 -w\n"
                        "\n"
                        "write -f blk.tmp\n"
                        "write -D 0 -f dump.tmp";

    
    QString working_directory;
    std::function<void()>  m_post_process = nullptr;
    QString execution_id = "";

public:
    GGrid_Process()
    {
        
        process = new QProcess();
        QObject::connect(process, &QProcess::readyReadStandardOutput, this, &GGrid_Process::readData);
        QObject::connect(process, &QProcess::finished, this, &GGrid_Process::process_finished);

        
    }
    ~GGrid_Process()
    {
        delete process;
        workerThread.quit();
        workerThread.wait();
    }

    void setExecutionId(QString exec_id)
    {
        execution_id = exec_id;
    }   
    void setOutputFrequency(int opf)
    {
        output_frequency = opf;
    }

    void setNumberofSweeps(int ns)
    {
        number_of_sweeps = ns;
    }

    void setGlobalDensity(double gd)
    {
        global_density = gd;
    }

    void setWorkingDirectory(const QString &wd)
    {
        working_directory = wd;
    }

    QProcess *process;

    void process_finished(int exitCode){
        gridpro_comms::post_packet("mainwindow", "stop_progress", {{"", ""}});
        std::map<QString, QString> data;
        if(success )
        {
            data["heading"] = "<span style = \"color:green;\">GGRID Execution Completed.</span>";
            data["message"] = "id: " + execution_id;
            
            //gridpro_comms::post_packet("mainwindow", "show_information", data);
        }
        else
        {
            if(!terminated)
                data["heading"] = "<span style = \"color:red;\">GGRID Execution Failed.</span>";
            else
                data["heading"] = "<span style = \"color:yellow;\">GGRID Process </span><span style = \"color:red;\">Terminated.</span>";
            
          
        }
        data["message"] = QString("Sweeps : " + swp + "/%1<br> No of Surface Folds: " + surf_folds).arg(number_of_sweeps);
        
        gridpro_comms::post_packet("mainwindow", "show_information", data);
        if(m_post_process)
            m_post_process();
        
    }

    void start(QStringList args)
    {
        QFile file(working_directory + "/topology_"+ execution_id + ".sch");
        file.open(QIODevice::WriteOnly | QIODevice::Text);
        QTextStream out(&file);
        QString gloden = "\n";
        
        if(global_density == 1)
        {
            gloden = "\n";
        }
        else if(global_density == 2)
        {
            gloden = "step 1: -g all 2.000000x\n";;
        }else if(global_density == 4)
        {
            gloden = "step 1: -g all 4.000000x\n";;
        }else if(global_density == 0.25)
        {
            gloden = "step 1: -g all 0.250000x\n";;
        }else if(global_density == 0.5)
        {
            gloden = "step 1: -g all 0.500000x\n";;
        }
        std::cout<<"global_density: "<<gloden.toStdString()<<std::endl;
        out <<schedule.arg(gloden).arg(std::ceil(number_of_sweeps/output_frequency)).arg(output_frequency);
        file.close();
        success = false;
        std::map<QString, QString> data;
        data["heading"] = "Starting GGRID";
        data["message"] = "Initializing.<br>Checking License.";
                gridpro_comms::post_packet("mainwindow", "show_information", data);
        process -> start("gp_utilities", args);
        terminated = false;
    }

    void send_info_to_gui(const std::string &header, const std::string &data)
    {
        auto p = gridpro_comms::create_packet("mainwindow", "gui_show_information");
        p -> set_data<std::string>("header", header);
        p -> set_data<std::string>("message", data);
        gridpro_comms::post(p);

    }

    void readData()
    {
        QString output = process -> readAllStandardOutput();
        output = output.trimmed();
        std::cout<<"[GGRID]: "<<output.toStdString()<<"\n----------------------------"<<std::endl;

        QFile file(working_directory + "/ggrid.log");
        file.open(QIODevice::WriteOnly  | QIODevice::Append  | QIODevice::Text);
        QTextStream out(&file);
        out <<output<<"\n";
        file.close();

        bool update = false;
        if(output.contains("swp"))
        {
            const int pos = output.lastIndexOf("swp");
            output = output.mid(pos).trimmed();
            swp = output.split(" ")[1];
            swp = QString::number(swp.toInt()+1);

            std::cout<<"[SWEEP]: "<<swp.toStdString()<<std::endl;
            
                update = true;
        }
        else if(output.startsWith("surf"))
        {
            
            auto foldslist = output.split("\n")[0].split(" ");
            surf_folds = foldslist[foldslist.size() - 2];
            std::cout<<"[FOLDS]: "<<surf_folds.toStdString()<<std::endl;
            update = true;
        }
        if(update)
        {
            std::map<QString, QString> data;
            data["heading"] = "<span style = \"color:yellow;\">GGRID</span>";
            data["message"] = QString("Sweeps : " + swp + "/%1<br> No of Surface Folds: " + surf_folds).arg(number_of_sweeps);
            if((swp.toInt())%output_frequency == 0)
            {
                data["message"] += "<br> Dumping results.";
            }
            
            gridpro_comms::post_packet("mainwindow", "show_information", data);
        }
        if(output.startsWith("End Grid Session"))
        {
            std::map<QString, QString> data;
            data["heading"] = "Finishing Up";
            data["message"] = QString("Sweeps : " + swp + "/%1<br> No of Surface Folds: " + surf_folds).arg(number_of_sweeps);
            
            gridpro_comms::post_packet("mainwindow", "show_information", data);
            success = true;
            
        }

    }

 

    template <typename T>
    void setPostProcessor( T *instance, void (T::*func)())
    {
         m_post_process = [instance, func]()
                                            {   
                                                if(func)
                                                    (instance->*func)();
                                            };
        
        return;

    }

    void terminate()
    {
        std::map<QString, QString> data;
        data["heading"] = "Terminating GGRID";
        data["message"] = QString("Attempting to terminate.");
        
        gridpro_comms::post_packet("mainwindow", "show_information", data);
        terminated = true;
        success = false;
        process -> terminate();
    }
};

class Generic_Process : public QObject
{
    Q_OBJECT
    QThread workerThread;

    bool success;
    bool terminated;
    QString title = "Process";
    QString working_directory;
    


    std::function<void(QString)>  m_process_console_output = nullptr;
    std::function<void()>  m_process_result = nullptr;
    std::function<void()>  m_process_function = nullptr;
    QProcess *process;
    Worker *worker;
   

public:
    Generic_Process()
    {
        
        process = new QProcess();
        QObject::connect(process, &QProcess::readyReadStandardOutput, this, &Generic_Process::readData);
        QObject::connect(process, &QProcess::finished, this, &Generic_Process::process_finished);

        
    }
    ~Generic_Process()
    {
        delete process;
        workerThread.quit();
        workerThread.wait();
    }


    void setWorkingDirectory(const QString &wd)
    {
        working_directory = wd;
    }
    bool isTerminated() const
    {
        return terminated;
    }

    
    template <typename T>
    void setProcessFunction(T *instance, void (T::*func)())
    {
         m_process_function = [instance, func]()
                                            {   
                                                if(func)
                                                    (instance->*func)();
                                            };
        
        return;

    }

    template <typename T>
    void setConsoleOutputProcessor(T *instance, void (T::*func)(QString data))
    {
         m_process_console_output = [instance, func](QString data)
                                            {   
                                                if(func)
                                                    (instance->*func)(data);
                                            };
        
        return;

    }

    template <typename T>
    void setPostProcessor(T *instance, void (T::*func)())
    {
         // m_process_result = [instance, func]()
         //                                    {   
         //                                        if(func)
         //                                            (instance->*func)();
         //                                    };

        Worker *worker = new Worker;
        worker->set_job("job", instance, func);
        
        return;

    }


    

    void process_finished(){
        std::cout<<"Finished generic process\n";
        gridpro_comms::post_packet("mainwindow", "stop_progress", {{"", ""}});
        if(m_process_result)
            m_process_result();
    }

    void start()
    {
        
        worker->moveToThread(&workerThread);
        connect(&workerThread, &QThread::finished, worker, &QObject::deleteLater);
        connect(this, &Generic_Process::operate, worker, &Worker::doWork);
        connect(worker, &Worker::resultReady, this, &Generic_Process::process_finished);
        workerThread.start();
    }

    void handleResults()
    {
        
    }

    void terminate()
    {
        terminated = true;
        process -> terminate();
    }

    void readData()
    {
        QString output = process -> readAllStandardOutput();
        if(m_process_console_output)
            m_process_console_output(output);
  
    }
    signals:
    void operate();
};