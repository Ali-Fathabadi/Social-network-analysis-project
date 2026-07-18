#include "Graph.h"
#include <iostream>


static void printFriends(const Graph& g, int id) {
    const User* u = g.getUser(id);
    if (!u) { std::cout << "کاربر یافت نشد\n"; return; }
    std::cout << "دوستان " << u->name << ": ";
    for (int fid : g.getFriends(id)) {
        std::cout << g.getUser(fid)->name << " ";
    }
    std::cout << "\n";
}

int main() {
    Graph g;

    // ساخت شبکه ی مثال موجود در سند پروژه
    g.addUser(1, "نیکا");
    g.addUser(2, "سارا");
    g.addUser(3, "لاله");
    g.addUser(4, "حسن");
    g.addUser(5, "امیر");
    g.addUser(6, "علی");
    g.addUser(7, "امید");
    g.addUser(8, "سعید");
    g.addUser(9, "مهدی");

    g.addFriendship(1, 2); 
    g.addFriendship(2, 3); 
    g.addFriendship(2, 4); 
    g.addFriendship(4, 5); 
    g.addFriendship(5, 6); 
    g.addFriendship(7, 8); 
    g.addFriendship(7, 9); 
    g.addFriendship(8, 9); 

    std::cout << "تعداد کاربران: " << g.userCount() << "\n";
    printFriends(g, 5); 

    // تست ذخیره سازی
    if (!g.saveToFile("network.json")) {
        std::cerr << "خطا در ذخیره فایل\n";
        return 1;
    }
    std::cout << "شبکه در network.json ذخیره شد.\n";

    // تست بازیابی در یک گراف جدید برای اطمینان از صحت ذخیره/بازیابی
    Graph g2;
    if (!g2.loadFromFile("network.json")) {
        std::cerr << "خطا در بازیابی فایل\n";
        return 1;
    }
    std::cout << "پس از بازیابی -> ";
    printFriends(g2, 5);

    // تست ویرایش و حذف
    g2.editUser(6, "علی رضایی");
    g2.removeUser(3); 
    std::cout << "پس از ویرایش/حذف، تعداد کاربران: " << g2.userCount() << "\n";

    return 0;
}
