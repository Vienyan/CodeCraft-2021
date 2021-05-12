# CodeCraft-2021
华为2021软件精英挑战赛源码 粤港澳赛区第9名

#### 成绩
最终是取得 粤港澳赛区第9名的成绩。很遗憾没有进入全球总决赛

#### 主要策略
我们一开始将整个赛题分成三个主要部分：
- 购买服务器时该如何选择购买何种服务器
- 如果现有服务器有多个能够放置，该如何选择
- 迁移策略
- 其实最后还有订单处理的策略

##### 购买新服务器策略
购买新服务器主要考虑成本问题，一开始我们采用的策略是：<br>
　　　　<b>cost = 每种服务器的建造成本 + 剩余天数 * 电费消耗成本</b><br>
按照这个从低到高排序，每次有add订单需要购买时，从低往高找，<b>一找到足够容量放置的就购买该种类型。</b><br>
但是复赛的时候发现，这种策略有一个问题就是购买的服务器种类非常单一。可能几万的订单处理下来，购买了6，7千台服务器，只有4，5种类型。因为这个排序基本是固定的，大容量的服务器一般成本都会比较高，但不代表其没用。有时候选择大容量的服务器能减少所需的服务器数量，以及在后面迁移时能够提高迁移成功概率。<br>
将排序方式添加一步：<br>
　　　　<b>cost = cost - ( 100 - 150*当前处于第几天/总天数 ) * (服务器的内存 + 服务器的cpu)</b> （当前所处天数 < 总天数的2/3）<br>

&emsp;&emsp;这样做的目的：将服务器的大小也纳入考虑之中。前期150*当前处于第几天/总天数比较小，则服务器容量大的，cost减到的部分也比较多，则有更大的机会被选到。100和150是用来调节的参数，因为cost比较大，而服务器的容量比较小，如果没用参数调节，减去的效果不明显。

##### 放置到现有服务器
&emsp;&emsp;遍历所有现有服务器，判断是否能够放置。如果能够放置的话，就计算<b>贴合程度</b>。最后取贴合程度最好的那个。
贴合程度计算：<br>
　　　<b>gap = 3 \*（服务器剩余cpu - 虚拟机所需cpu）+  服务器剩余内存 - 虚拟机所需虚拟机</b>
　　　<b>gap = gap \* (5 * 将该虚拟机插入服务器后cpu和内存利用率的差/0.1)</b><br>
第一个公式说明：如果插入之后剩下的空间越多，则说明贴合程度越差。cpu的系数3是试验出来的。好像用这个系数效果好一些<br>
第二个公式说明：如果放置进去之后造成服务器cpu和内存利用率失衡，则说明贴合程度也比较差。如果放置之后，该节点cpu用满100%，而内存只用了90%，则gap要乘以5倍。<br>
<b>因为有迁移完之后，可能会出现一些空的服务器，这些服务器应该尽量不使用，减少电费消耗</b>。如果为空服务器，也应该给gap一定惩罚。并且这个与服务器的电费消耗有关，电费越高越不应该选。<br> 
　　　<b>gap = gap \* 5 +  服务器的电费</b><br>


##### 迁移策略
&emsp;&emsp;我们采用了两层迁移，第一层迁移是将所有服务器按照电费从高到低排序，大概取电费最高的6%的服务器，迁移到电费低的服务器上。这样做的目的是，尽量空出电费高的服务器，降低电费支出成本。<br>
在电费迁移之后，再将服务器按照平均利用率进行排序，从利用率低的服务器往利用率高的服务器迁移。

##### 订单处理策略
&emsp;&emsp;按照正常情况，在输入一笔订单之后，就可以进行处理了。但是我们这里利用了题目输入的特点，题目中每天的订单可以都获取完之后再进行处理。这样就可以进行一定操作。<b>不按照输入顺序处理订单是否可行呢？</b>其实只要不出现delete，那么add订单的处理顺序是不会影响结果的正确性的。因为不管顺序如何，add订单总是要找到位置存放的，只要能够按顺序输出对应的存储位置即可。<br>
　　所以我们<b>将每天的订单根据delete划分成多个组，每个组内add订单都是按照虚拟机的大小（cpu + 内存）从大到小排序</b>，也就是先处理比较大的虚拟机。


&emsp;&emsp;在对订单排完序之后，可以另外思考，<b>一定要每次遇到一个放置不了的就买一台服务器吗？</b>其实也不用，因为如果一个虚拟机无法放入现有的服务器，则必然要买新的服务器，如果有多个这样的虚拟机组合起来，一起购买也是可以的。这样做的好处就是：本来可能需要购买2、3个服务器的，但现在我们只需要买一台。
　　所以我们<b>将当前无法放置的虚拟机先收起来，当有3个这样的虚拟机时，再一起购买服务器。</b>此时尽量购买一台服务器，能够同时容纳3台虚拟机。<br>

&emsp;&emsp;另外,我们还采用了一个小策略。在对每日订单排序的时候，我们能够计算今天所有add订单所需要的cpu和内存，根据这个计算，可以得到今天需要cpu多一些还是内存多一些。因此就在购买服务器的时候，可以有意地将今天所需要的类型买多一些。例如今天比较需要cpu多的，那么在购买时买cpu大一些的服务器，有利于为后续订单提供一个合适的空间。
