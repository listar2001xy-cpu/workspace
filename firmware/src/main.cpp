#include <Arduino.h>

#include "app/app_state.h"

// 薄入口：仅负责构造 app 并委托 setup()/loop()，不承载业务逻辑。
AppState app;

void setup() {
    app.setup();
}

void loop() {
    app.loop();
}
