## Plan: 支持贴图材质

TL;DR - 在现有架构上新增一个最小 `Texture` 功能并把它接入 `Material`：加载图片、在材质中保存纹理指针、在三角形/网格处插值 UV 并把 uv 写入 `Intersection::tcoords`，最后在 `Material::getColorAt(u,v)` 返回贴图颜色并用于 BRDF 计算。

**Steps**
1. 补完已有的纹理类：`Texture.hpp`（使用 `stb_image.h` 加载图片，支持宽高、像素数据、获取像素）
2. 在 `Material` 中添加 `Texture* tex` 字段并实现 `getColorAt(u,v)`：当 `tex!=nullptr` 时返回贴图采样颜色，否则返回 `Kd`。*parallel with step 3*
3. 确保三角形/网格保存顶点 UV，并在交点处将插值后的 UV 写入 `Intersection::tcoords`（修改 `Triangle::getSurfaceProperties` / `Triangle::getIntersection` 或在 `Triangle::getIntersection` 成功时计算并设置 `tcoords`）。*depends on step 2*
5. 在 `main.cpp`（或场景构造处）加载贴图并把 `Texture*` 赋给 `Material->tex`；例如 `Material* m = new Material(...); m->tex = new Texture("diffuse.png");`*depends on step 1,2,4*
6. 在 `Material::eval/sample/pdf` 中当调用 `getColorAt(u,v)`（或在 `eval` 中使用 `Kd = getColorAt(...)`）使贴图影响最终 BRDF 价值（注意对发光材质使用 `getEmission()` 单独处理）。*depends on step 2,3*


**Relevant files**
- `/home/endlia/SED/Material.hpp` — 添加 `Texture* tex`、实现 `getColorAt(u,v)` 并在 `eval`/`sample` 中使用
- `/home/endlia/SED/Texture.hpp` (新增) — 声明 `Texture` 类成员与采样方法
- `/home/endlia/SED/Triangle.hpp` — 使用 OBJ 中的纹理坐标，插值并在 `Intersection.tcoords` 中写入
- `/home/endlia/SED/MeshTriangle.hpp` — 在构造时把 OBJ 的 `map_Kd` 或顶点纹理坐标传递/保留，`Sample()`/`getIntersection()` 要把 `pos.tcoords`/`inter.tcoords` 填上
- `/home/endlia/SED/OBJ_Loader.hpp` — 已有 `map_Kd` 解析，检查并暴露给 `MeshTriangle` 使用
- `/home/endlia/SED/main.cpp` — 在场景中加载纹理文件并赋给 `Material`


**Decisions / 假设**
- 采用简单、最小侵入式实现：`Texture*` 使用裸指针以匹配当前代码风格（可改为 `std::shared_ptr<Texture>`）。
- 使用 `stb_image.h`（单头文件）加载 PNG/JPG；不实现 MIPMAP 或高级纹理过滤，仅双线性过滤可选。
- 将 UV 坐标假定为 [0,1]；对超出范围使用重复（wrap）或 clamp 可作为配置项。