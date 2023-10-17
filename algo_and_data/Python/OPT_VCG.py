import docplex.mp.model as cpx
import pandas as pd
import time
from paper.experiment_data import user_size, res_size, bids, caps, S

# user_size, res_size = 3, 3
#
# bids = {1: 12, 2: 5, 3: 4}
# caps = {1: 5, 2: 5, 3: 5}
# S = {(1, 1): 3, (1, 2): 1, (1, 3): 2,
#      (2, 1): 2, (2, 2): 1, (2, 3): 1,
#      (3, 1): 1, (3, 2): 1, (3, 3): 1}


alloc_ret, pay, social_welfare, total_pay = None, [0.0] * user_size, 0.0, 0.0

set_I, set_R = range(1, user_size + 1), range(1, res_size + 1)

start_time, end_time = None, None


def opt_alloc():
    # 创建模型
    opt_model = cpx.Model(name="Binary Model")
    # 定义决策变量
    x_vars = {i: opt_model.binary_var(name="x_{0}".format(i)) for i in set_I}
    # 添加约束条件
    constraints = {r: opt_model.add_constraint(
        ct=opt_model.sum(S[i, r] * x_vars[i] for i in set_I) <= caps[r],
        ctname="constraint_{0}".format(r)) for r in set_R}
    # 定义目标函数
    objective = opt_model.sum(bids[i] * x_vars[i] for i in set_I)
    # 最大化目标
    opt_model.maximize(objective)
    # 求解模型
    sol = opt_model.solve()
    # print(sol, end='')

    # 将Cplex求解的结果转换成list并返回
    opt_df = pd.DataFrame.from_dict(x_vars, orient="index", columns=["variable_object"])
    opt_df["solution_value"] = opt_df["variable_object"].apply(lambda item: item.solution_value)
    opt_df.drop(columns=["variable_object"], inplace=True)
    tmp_alloc = opt_df['solution_value'].tolist()
    # print(tmp_alloc)
    # print()
    return tmp_alloc


def opt_vcg():
    global alloc_ret, pay, total_pay
    for i in range(user_size):
        if alloc_ret[i]:    # 第i + 1个用户胜出了!
            tmp_bid = bids[i + 1]
            bids[i + 1] = -1000

            # print("踢出第" + str(i + 1) + "个用户后，分配结果为：")
            tmp_alloc = opt_alloc()

            bids[i + 1] = tmp_bid
            social_welfare_except_i = 0.0
            for j in range(user_size):
                if tmp_alloc[j]:
                    social_welfare_except_i += bids[j + 1]
            pay[i] = round(social_welfare_except_i - (social_welfare - bids[i + 1]))
            total_pay += pay[i]


def opt_alloc_and_vcg():
    global alloc_ret, social_welfare, start_time, end_time
    start_time = time.perf_counter()
    # 包含所有用户时的最优分配
    tmp_alloc = opt_alloc()
    alloc_ret = [x for x in tmp_alloc]
    for i in range(user_size):
        if alloc_ret[i]:
            social_welfare += bids[i + 1]
    opt_vcg()
    end_time = time.perf_counter()
    show_info()


def calc_rur():
    allocated_res = [0] * res_size
    for i in range(user_size):
        if alloc_ret[i]:
            for j in range(res_size):
                allocated_res[j] += S[(i + 1, j + 1)]
    original_res = list(caps.values())[:res_size]
    # print('################################################')
    # print(allocated_res)
    # print(original_res)
    # print('################################################')
    return [allocated_res[i] / original_res[i] for i in range(res_size)]


def show_info():
    print("分配结果：", alloc_ret)
    print("支付结果：", pay)
    print()
    print("用户总支付：%f, 社会福利：%f" % (total_pay, round(social_welfare)))
    print("资源利用率：", calc_rur())


print(sum(list(bids.values())))
opt_alloc_and_vcg()
execute_time = end_time - start_time
print('执行时间为：%fs' % execute_time)












# print(opt_alloc())
# bids[1] = 0
# bids[2] = 0
# bids[3] = 0
# bids[4] = 0
# bids[5] = 0
# bids[6] = 0
# print(opt_alloc())

