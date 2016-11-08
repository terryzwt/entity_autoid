
>Inspired by 

https://github.com/fleeto/autoid

> Just a tool for PHP coders.

#Auto ID

A Drupal module for generating token-based serial numbers, 
for example `NODE201502280001`.




##Usage

Install the module, Then you can find configuration link in `admin/config`.

You can define new schema in the config page.

At last , you can use `entity_autoid_get_serial($entity_type, $entity)` 
in your **PHP code** to generate a new serial number.


---
> 受看山所写模块启发：
https://github.com/fleeto/autoid
> 模块流请忽略本模块 

#流水号

用于生成流水号（例如`NODE201502280001`）的一个简单模块。

##用法

安装模块，在配置页面能看到该模块的配置入口。

可以在配置页面生成新的ID样式。

接下来就可以在**代码中**使用`entity_autoid_get_serial($entity_type, $entity)`的形式来生成新的流水号了
