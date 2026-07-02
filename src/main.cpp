#include <QApplication>
#include <QLabel>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    QLabel placeholder(QStringLiteral("micran-tree-cache"));
    placeholder.setMinimumSize(400, 100);
    placeholder.setAlignment(Qt::AlignCenter);
    placeholder.show();

    return app.exec();
}