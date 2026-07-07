#include "application/CacheService.h"
#include "infrastructure/DatabaseRepository.h"
#include "infrastructure/InMemoryDatabase.h"
#include "presentation/mainwindow.h"

#include <QApplication>

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    micran_tree_cache::infrastructure::InMemoryDatabase database;
    micran_tree_cache::infrastructure::DatabaseRepository repository{database};

    micran_tree_cache::application::CacheService cache{repository};

    micran_tree_cache::presentation::MainWindow window{cache, repository, database};
    window.show();

    return app.exec();
}