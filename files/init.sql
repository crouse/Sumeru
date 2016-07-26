
CREATE DATABASE `vol` /*!40100 DEFAULT CHARACTER SET latin1 */;

use vol;

/* 部门表 */
CREATE TABLE `depart` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `name` varchar(45) NOT NULL COMMENT '部组名',
  `level` int(4) NOT NULL COMMENT '部组级别',
  `topper` varchar(64) DEFAULT NULL COMMENT '上级部组名称',
  `startupdate` varchar(45) NOT NULL COMMENT '成立日期',
  `enddate` varchar(45) NOT NULL COMMENT '撤销日期',
  `master` varchar(45) NOT NULL COMMENT '主管法师名字',
  `incharge` varchar(45) NOT NULL COMMENT '负责人名',
  `phone` varchar(45) DEFAULT NULL COMMENT '负责人手机号',
  `tel` varchar(45) DEFAULT NULL COMMENT '部组分机号码',
  `stat` varchar(10) DEFAULT NULL COMMENT '部组状态',
  `notes` varchar(512) DEFAULT NULL COMMENT '其他记录',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=utf8;


/* 学习/工作表 */
CREATE TABLE `edujob` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `pid` varchar(45) NOT NULL COMMENT '义工身份证号码',
  `start` varchar(45) DEFAULT NULL COMMENT '开始年份',
  `end` varchar(45) DEFAULT NULL COMMENT '结束年份',
  `place` varchar(45) NOT NULL COMMENT '学校或单位名称',
  `notes` varchar(512) DEFAULT NULL COMMENT '其他记录',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=utf8;


/* 家庭信息表 */
CREATE TABLE `family` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `vpid` varchar(45) NOT NULL COMMENT '义工身份证号码',
  `name` varchar(45) NOT NULL COMMENT '义工家庭成员姓名',
  `birthday` varchar(45) DEFAULT NULL COMMENT '生日',
  `relation` varchar(45) NOT NULL COMMENT '关系',
  `address` varchar(128) DEFAULT NULL COMMENT '住址',
  `phone` varchar(45) DEFAULT NULL COMMENT '手机号码',
  `health` varchar(45) DEFAULT NULL COMMENT '健康状况',
  `faith` varchar(45) DEFAULT NULL COMMENT '信仰情况',
  `notes` varchar(512) DEFAULT NULL COMMENT '其他记录',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=utf8;


/* 图像存放表 */
CREATE TABLE `images` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `pid` varchar(45) NOT NULL COMMENT '身份证号',
  `image` mediumblob COMMENT '图像的二进制文件存放',
  `uptime` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP COMMENT '插入图片时间',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=utf8;

/* 操作记录表 */
CREATE TABLE `ophistory` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `name` varchar(45) NOT NULL COMMENT '行为',
  `json` varchar(45) NOT NULL COMMENT '操作记录',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

/* 用户管理表 */
CREATE TABLE `sumeru_user` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `username` varchar(45) NOT NULL COMMENT '用户名称',
  `password` varchar(45) NOT NULL COMMENT '用户密码哈希值',
  `mac` varchar(45) NOT NULL COMMENT '用户硬件网卡地址',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=utf8;

/* 调动信息表 */
CREATE TABLE `transfer` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `pid` varchar(45) NOT NULL COMMENT '义工身份证号码',
  `start` datetime DEFAULT NULL COMMENT '开始日期',
  `depart` varchar(45) NOT NULL COMMENT '一级部组',
  `departsecond` varchar(45) NOT NULL COMMENT '二级部组',
  `notes` varchar(512) DEFAULT NULL COMMENT '其他记录',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=utf8;


/* 义工档案表 */
CREATE TABLE `volunteer` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `name` varchar(45) NOT NULL COMMENT '姓名',
  `gender` varchar(10) DEFAULT NULL COMMENT '性别',
  `birthday` varchar(45) DEFAULT NULL COMMENT '生日',
  `phone` varchar(45) DEFAULT NULL COMMENT '手机号码',
  `home` varchar(128) DEFAULT NULL COMMENT '籍贯',
  `nowhome` varchar(45) DEFAULT NULL COMMENT '现居地址',
  `graduate` varchar(45) DEFAULT NULL COMMENT '毕业学校',
  `academic` varchar(45) DEFAULT NULL COMMENT '学历',
  `field` varchar(45) DEFAULT NULL COMMENT '专业',
  `workplace` varchar(45) DEFAULT NULL COMMENT '现工作单位',
  `job` varchar(45) DEFAULT NULL COMMENT '职务',
  `hobby` varchar(45) DEFAULT NULL COMMENT '技术特长',
  `ifguiyi` varchar(45) DEFAULT NULL COMMENT '是否皈依',
  `code` varchar(45) DEFAULT NULL COMMENT '皈依证号',
  `ifgroup` varchar(45) DEFAULT NULL COMMENT '是否已参加学佛小组',
  `iffzzj` varchar(45) DEFAULT NULL COMMENT '是否是佛子之家成员',
  `pid` varchar(45) DEFAULT NULL COMMENT '身份证号码',
  `emer` varchar(45) DEFAULT NULL COMMENT '紧急联系人',
  `emerphone` varchar(45) DEFAULT NULL COMMENT '紧急联系人手机号',
  `startlearnbud` varchar(45) DEFAULT NULL COMMENT '学佛始年',
  `licenceplate` varchar(45) DEFAULT NULL COMMENT '车牌号码',
  `email` varchar(45) DEFAULT NULL COMMENT '邮箱',
  `staydate` datetime DEFAULT NULL COMMENT '常住日期',
  `health` varchar(45) DEFAULT NULL COMMENT '健康状况',
  `race` varchar(45) DEFAULT NULL COMMENT '民族',
  `marriage` varchar(45) DEFAULT NULL COMMENT '婚姻状况',
  `depart` varchar(128) DEFAULT NULL COMMENT '一级部门名称',
  `departsecond` varchar(128) DEFAULT NULL COMMENT '二级部门名称',
  `past` varchar(512) DEFAULT NULL COMMENT '学佛历程',
  `whyhere` varchar(512) DEFAULT NULL COMMENT '来山上的因缘',
  `hopefor` varchar(512) DEFAULT NULL COMMENT '来山上有什么希求',
  `stat` varchar(12) NOT NULL COMMENT '义工状态',
  `voltype` varchar(32) NOT NULL COMMENT '义工类型',
  `notes` varchar(512) DEFAULT NULL COMMENT '其他记录',
  PRIMARY KEY (`id`),
  UNIQUE KEY `pid_UNIQUE` (`pid`)
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=utf8;


