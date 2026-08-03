#pragma once
#include "Graph.h"
#include <string>
#include <vector>

namespace algo {

// انتخاب K کاربر بهینه برای شروع انتشار خبر:
//  اولویت اول: بیشترین تعداد افراد شبکه در نهایت خبر را دریافت کنند
//  اولویت دوم: خبر در کمترین زمان ممکن به آن افراد برسد
std::vector<std::string> optimizeNewsSpread(const Graph& g, int k);

}
