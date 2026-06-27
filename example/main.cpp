#include <QApplication>
#include <QMainWindow>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QComboBox>
#include <QThread>
#include <QTimer>
#include <cmath>
#include <atomic>
#include <memory>
#include <QPlainTextEdit>

#include "RealtimePlot.h"

// ---------------------------------------------------------------------------
// Simulated data-acquisition thread (mimics STM32 USB CDC data stream)
// ---------------------------------------------------------------------------
class DataThread : public QThread
{
    Q_OBJECT
public:
    std::shared_ptr<PlotSeries> ch1, ch2, ch3;
    std::atomic<bool> running{true};

    void run() override
    {
        double t = 0.0;
        const double dt = 1.0 / 1000.0; // 1 kHz sample rate

        while (running)
        {
            // Push a burst of 10 samples (simulate USB packet)
            for (int i = 0; i < 100; ++i)
            {
                double v1 = std::sin(2 * M_PI * 10.0 * t) + 0.3 * std::sin(2 * M_PI * 53.7 * t);
                double v2 = 5 * std::cos(2 * M_PI * 5.0 * t / 100) * std::exp(-0.1 * std::fmod(t, 3.0));
                double v3 = 0.5 * std::sin(2 * M_PI * 1.0 * t) + 0.1 * (static_cast<double>(rand()) / RAND_MAX - 0.5);

                ch1->pushPoint(t, v1);
                ch2->pushPoint(t, v2);
                ch3->pushPoint(t, v3);
                t += dt;
            }

            emit newData();
            // Sleep ~10 ms between bursts → ~100 bursts/s → 1000 pts/s total
            QThread::msleep(5);
        }
    }

signals:
    void newData();
};

// ---------------------------------------------------------------------------
// Main window
// ---------------------------------------------------------------------------
class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow()
    {
        setWindowTitle("RealtimePlot Demo – Qt/OpenGL");
        resize(1000, 600);

        // --- Central widget ---
        auto *central = new QWidget(this);
        auto *vlay = new QVBoxLayout(central);
        vlay->setContentsMargins(4, 4, 4, 4);
        vlay->setSpacing(4);
        setCentralWidget(central);

        // --- Text Info ---
        m_text = new QPlainTextEdit(this);
        m_text->setReadOnly(true);
        vlay->addWidget(m_text);

        // --- Plot ---
        m_plot = new RealtimePlot(this);
        m_plot->setTitle("Real-time ADC Channels (simulated @ 1 kHz)");
        m_plot->setXLabel("Time (s)");
        m_plot->setYLabel("Voltage (V)");
        m_plot->setRefreshRate(60);

        ch1 = m_plot->addSeries("Ch1 - 10 Hz sine", QColor(0, 210, 255), 5000);
        ch2 = m_plot->addSeries("Ch2 - 5 Hz decaying", QColor(255, 180, 0), 5000);
        ch3 = m_plot->addSeries("Ch3 - 1 Hz + noise", QColor(80, 255, 120), 5000);
        m_plot->start();

        vlay->addWidget(m_plot, 1);

        // --- Controls ---
        auto *hlay = new QHBoxLayout;
        vlay->addLayout(hlay);

        auto *lblZoom = new QLabel("Zoom mode:", this);
        auto *cmbZoom = new QComboBox(this);
        cmbZoom->addItems({"XY", "X only", "Y only"});
        connect(cmbZoom, QOverload<int>::of(&QComboBox::currentIndexChanged),
                this, [this](int idx)
                { m_plot->setZoomMode(static_cast<RealtimePlot::ZoomMode>(idx)); });

        auto *btnFit = new QPushButton("Auto-fit [F]", this);
        btnFit->setCheckable(true);
        btnFit->setChecked(true);

        auto *btnGrid = new QPushButton("Grid", this);

        auto *lblHelp = new QLabel(
            "<small>🖱 Wheel = zoom  |  Left drag = pan  |  Right drag = box-zoom  |  Dbl-click = fit</small>",
            this);
        lblHelp->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

        hlay->addWidget(lblZoom);
        hlay->addWidget(cmbZoom);
        hlay->addSpacing(12);
        hlay->addWidget(btnFit);
        hlay->addWidget(btnGrid);
        hlay->addStretch();
        hlay->addWidget(lblHelp);

        connect(btnGrid, &QPushButton::clicked, this, [this](bool)
                { this->m_plot->setGrid({!this->m_plot->getGrid().enabled, this->m_plot->getGrid().color}); });

        connect(btnFit, &QPushButton::clicked, m_plot, [this](bool a)
                { this->m_plot->setAutoZoom(a); });

        // --- Data thread ---
        m_thread = new DataThread;
        m_thread->ch1 = ch1;
        m_thread->ch2 = ch2;
        m_thread->ch3 = ch3;
        m_thread->start();

        connect(m_thread, &DataThread::newData, this, &MainWindow::newDataArrived);
    }

    ~MainWindow() override
    {
        m_thread->running = false;
        m_thread->wait();
        delete m_thread;
    }

    void newDataArrived()
    {

        PlotSeries::IntervalStats stats = ch1->calculateIntervalStats(m_plot->getCursorX(RealtimePlot::CursorType::X1), m_plot->getCursorX(RealtimePlot::CursorType::X2));
        m_text->setPlainText(QString("Mean: %1").arg(stats.mean));
        m_text->appendPlainText(QString(" Std: %1").arg(stats.rms));
    }

private:
    RealtimePlot *m_plot = nullptr;
    DataThread *m_thread = nullptr;
    QPlainTextEdit *m_text = nullptr;
    std::shared_ptr<PlotSeries> ch1, ch2, ch3;
};

// ---------------------------------------------------------------------------
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setStyle("Fusion");

    // Dark palette
    QPalette dark;
    dark.setColor(QPalette::Window, QColor(30, 30, 35));
    dark.setColor(QPalette::WindowText, QColor(200, 200, 210));
    dark.setColor(QPalette::Button, QColor(45, 45, 52));
    dark.setColor(QPalette::ButtonText, QColor(200, 200, 210));
    dark.setColor(QPalette::Base, QColor(22, 22, 28));
    dark.setColor(QPalette::Highlight, QColor(0, 150, 210));
    app.setPalette(dark);

    MainWindow w;
    w.show();
    return app.exec();
}

#include "main.moc"
