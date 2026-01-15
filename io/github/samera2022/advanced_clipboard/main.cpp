#include "main.h"
#include <QApplication>
#include <QMenu>
#include <QCloseEvent>
#include <QMimeData>
#include <QImage>

AdvancedClipboard::AdvancedClipboard(QWidget *parent)
        : QMainWindow(parent),
          historyListWidget(new QListWidget(this)),
          trayIcon(new QSystemTrayIcon(this)),
          clipboard(QApplication::clipboard()) {

    setupUI();
    setupTrayIcon();

    // Connect the clipboard's dataChanged signal to our slot
    connect(clipboard, &QClipboard::dataChanged, this, &AdvancedClipboard::onClipboardChanged);
}

void AdvancedClipboard::setupUI() {
    // Use the list widget as the central widget of the main window
    setCentralWidget(historyListWidget);
    // Set window properties to make it look like a popup
    setWindowFlags(Qt::FramelessWindowHint | Qt::Popup | Qt::WindowStaysOnTopHint);
    resize(300, 400); // Default size
}

void AdvancedClipboard::setupTrayIcon() {
    // Create a context menu for the tray icon
    QMenu *trayMenu = new QMenu(this);
    QAction *quitAction = trayMenu->addAction("Exit");
    connect(quitAction, &QAction::triggered, qApp, &QApplication::quit);

    trayIcon->setContextMenu(trayMenu);
    // TODO: Use a proper icon file
    trayIcon->setIcon(this->style()->standardIcon(QStyle::SP_ComputerIcon));
    trayIcon->setToolTip("Advanced Clipboard");
    trayIcon->show();
}

void AdvancedClipboard::onClipboardChanged() {
    const QMimeData *mimeData = clipboard->mimeData();
    if (!mimeData) {
        return;
    }

    // Create a QVariant to store the data
    QVariant data;
    if (mimeData->hasImage()) {
        data = mimeData->imageData();
    } else if (mimeData->hasText()) {
        data = mimeData->text();
    } else if (mimeData->hasUrls()) {
        // For files/folders, store their paths
        QStringList paths;
        for (const QUrl &url : mimeData->urls()) {
            paths.append(url.toLocalFile());
        }
        data = paths.join("\n");
    } else {
        // Unsupported type for now
        return;
    }

    // Avoid adding duplicates
    if (!clipboardHistory.isEmpty() && clipboardHistory.first() == data) {
        return;
    }

    // Add to history and trim if necessary
    clipboardHistory.prepend(data);
    if (clipboardHistory.size() > MAX_HISTORY_SIZE) {
        clipboardHistory.removeLast();
    }

    // Update the UI list
    historyListWidget->clear();
    for (const QVariant &item : clipboardHistory) {
        if (item.type() == QVariant::Image) {
            historyListWidget->addItem(new QListWidgetItem(QIcon(item.value<QImage>().scaled(32, 32)), "[Image]"));
        } else {
            historyListWidget->addItem(item.toString().section('\n', 0, 0)); // Show first line
        }
    }
}

void AdvancedClipboard::closeEvent(QCloseEvent *event) {
    // When the user tries to close the window, just hide it
    if (isVisible()) {
        hide();
        event->ignore();
    }
}
