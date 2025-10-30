#include <iostream>

#include <iostream>
#include <iomanip>
#include <locale>
#include <cmath>
#include <cstdlib>
#include <ctime>

using namespace std;

// === КОНСТАНТЫ ===
const double INIT_ACCOUNT = 10000.0;
const double BASIC_PRICE = 30.0;
const double OFFER_BASE_PRICE = 35.0;
const double MAX_DEMAND = 100.0;
const double MEAN_D_PRICE = 100.0;
const double DAILY_SPENDING = 700.0;
const double RENT_RATE = 200.0;
const double WAGES_AND_TAXES = 500.0;
const double VAT_RATE = 0.2;
const double OFFER_VOLUME_BASE = 40.0;
const double BASIC_STORE_INIT = 360.0;
const double SHOP_STORE_INIT = 80.0;
const int SIMULATION_DAYS = 100;

// === НАЛОГИ ===
const double PROFIT_TAX_RATE = 0.20;     // 20% налог на прибыль
const int TAX_PAYMENT_DAY = 30;          // Каждые 30 дней

// === СОСТОЯНИЕ ===
struct ModelState {
    double account;
    double basicStore;
    double shopStore;
    double income;
    double demand;
    double retPrice;
    double offerVolume;
    double offerPrice;
    double offerAccept;
    double transferVol;
    double sold;
    double lost;
    double totalRevenue;     // Накопленный доход
    double totalExpenses;    // Накопленные расходы
    double taxBase;          // Налоговая база
    double taxToPay;         // Налог к уплате
    int day;
};

// === ВЫВОД ===
void printState(const ModelState& state) {
    cout << "\n";
    cout << "========================================\n";
    cout << "           ДЕНЬ " << state.day << " из " << SIMULATION_DAYS << "\n";
    cout << "========================================\n";
    cout << fixed << setprecision(2);
    cout << "Счёт:                    " << state.account << " руб.\n";
    cout << "Склад (база):            " << state.basicStore << " ед.\n";
    cout << "Склад (магазин):         " << state.shopStore << " ед.\n";
    cout << "Спрос:                   " << state.demand << " ед.\n";
    cout << "Цена:                    " << state.retPrice << " руб.\n";
    cout << "Доход:                   " << state.income << " руб.\n";

    cout << "\n--- Мелкоопт ---\n";
    cout << "Объём:                   " << state.offerVolume << " ед.\n";
    cout << "Цена:                    " << state.offerPrice << " руб.\n";

    cout << "\n--- НАЛОГИ (за " << TAX_PAYMENT_DAY << " дней) ---\n";
    cout << "Доход всего:             " << state.totalRevenue << " руб.\n";
    cout << "Расходы всего:           " << state.totalExpenses << " руб.\n";
    cout << "Налоговая база:          " << state.taxBase << " руб.\n";
    cout << "Налог к уплате:          " << state.taxToPay << " руб.\n";
    cout << "----------------------------------------\n";
}

// === ВВОД ===
void inputStep(ModelState& state) {
    cout << "\nВВОД НА ДЕНЬ " << state.day << ":\n";
    cout << "----------------------------------------\n";
    double vol;
    cout << "Перевозка (0 = нет): ";
    cin >> vol;
    state.transferVol = max(0.0, min(vol, state.basicStore));

    int accept;
    cout << "Принять партию? (1/0): ";
    cin >> accept;
    state.offerAccept = (accept == 1) ? 1.0 : 0.0;

    cout << "Цена продажи: ";
    cin >> state.retPrice;
    state.retPrice = max(10.0, min(state.retPrice, 50.0));
}

int main() {
    setlocale(LC_ALL, "Russian");
    srand(time(0));

    cout << "=======================================\n";
    cout << "   МАГАЗИН С НАЛОГАМИ (30 дней)\n";
    cout << "=======================================\n\n";

    ModelState state = { INIT_ACCOUNT, BASIC_STORE_INIT, SHOP_STORE_INIT, 0, 0, 15.0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

    for (state.day = 1; state.day <= SIMULATION_DAYS; ++state.day) {

        if (state.day % 10 == 1) {
            state.offerVolume = OFFER_VOLUME_BASE + (rand() % 20 - 10);
            state.offerVolume = max(30.0, min(state.offerVolume, 50.0));
            state.offerPrice = OFFER_BASE_PRICE + (rand() % 10 - 5);
            state.offerPrice = max(30.0, min(state.offerPrice, 40.0));
        }

        printState(state);
        if (state.day < SIMULATION_DAYS) inputStep(state);

        // Перевозка
        if (state.transferVol > 0) {
            state.basicStore -= state.transferVol;
            state.shopStore += state.transferVol;
        }

        // Покупка партии
        if (state.offerAccept == 1.0) {
            double cost = state.offerVolume * state.offerPrice;
            if (state.account >= cost) {
                state.account -= cost;
                state.basicStore += state.offerVolume;
                state.totalExpenses += cost;
            }
            else {
                cout << "Не хватает денег!\n";
                state.offerAccept = 0;
            }
        }

        // Спрос и продажа
        state.demand = MAX_DEMAND * (MEAN_D_PRICE / state.retPrice);
        state.demand = max(0.0, state.demand + (rand() % 40 - 20));
        double canSell = min(state.shopStore, state.demand);
        state.sold = canSell;
        state.shopStore -= canSell;
        state.income = canSell * state.retPrice;
        state.account += state.income;
        state.totalRevenue += state.income;

        // Расходы
        double dailyCost = DAILY_SPENDING + RENT_RATE + WAGES_AND_TAXES;
        state.account -= dailyCost;
        state.totalExpenses += dailyCost;

        // === НАЛОГИ: каждые 30 дней ===
        if (state.day % TAX_PAYMENT_DAY == 0) {
            state.taxBase = state.totalRevenue - state.totalExpenses;
            if (state.taxBase > 0) {
                state.taxToPay = state.taxBase * PROFIT_TAX_RATE;
                if (state.account >= state.taxToPay) {
                    state.account -= state.taxToPay;
                    cout << "УПЛАЧЕН НАЛОГ: " << state.taxToPay << " руб.\n";
                }
                else {
                    cout << "НЕ ХВАТАЕТ НА НАЛОГ!\n";
                }
            }
            else {
                state.taxToPay = 0;
                cout << "Убыток — налог не платится.\n";
            }
            // Сброс накопителей
            state.totalRevenue = 0;
            state.totalExpenses = 0;
        }

        if (state.day < SIMULATION_DAYS) {
            cout << "\nНажмите Enter...\n";
            cin.ignore(); cin.get();
        }
    }

    cout << "\n\n=======================================\n";
    cout << "ИТОГ: Счёт = " << state.account << " руб.\n";
    cout << "Осталось неуплаченного налога: " << state.taxToPay << " руб.\n";
    cout << "=======================================\n";

    return 0;
}
