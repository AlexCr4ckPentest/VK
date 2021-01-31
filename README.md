# VK
VK API Lib - небольшая библиотека для работы с vk.com API, написанная на чистом C++. Из зависимостей требуется только Curl.
Библиотека предоставляет базовый класс `VK::Client`, с методами авторизации и выполнения запросов к API.

Есть поддержка:

* Обработки капчи

* Двух-факторной авторизации

На основе его Вы можете создавать свои классы для работы с разделами, для примера приведен класс `VK::Messages`.

Базовый пример использования:
```cpp
#include <iostream>
#include "src/api.h"

int main()
{
    VK::Client api{};

    if (!api.auth(login, pass, access_token)) {
        std::cout << "Auth fail\n";
        return 1;
    }

    std::cout << "Auth ok\n";
    std::cout << "Access token: " << api.access_token() << "\n";  
    std::cout << api.call("wall.post", "owner_id=12345&message=Test");

    return 0;
}
```
