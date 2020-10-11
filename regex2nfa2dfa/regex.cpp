#pragma warning (disable:4996)
#include <iostream>
#include <algorithm>
#include <iomanip>
#include <cstring>
#include <string>
#include <cstdio>
#include <cmath>
#include <vector>
#include <stack>
#include <queue>
#include <map>
#include <set>
#define inf 0X3f3f3f3f
using namespace std;
typedef long long ll;
typedef unsigned long long ull;

/*状态集合为0~n-1的数字(n为状态个数)，-1表示无指向*/
/*字符集合只包含a~z(97~122)的英文字符，空串ε表示为ascii:96 */

struct edge {
	char c; //转移字符
	int nex; //下一个状态的状态编号
	edge(char ch, int n) {
		c = ch, nex = n;
	}
	bool isEmpty() {
		return c == 96;
	}
};

struct state {
	int stateID;//状态编号ID
	vector<edge> trans;//转移状态(边)
	bool isBegin;//是否为开始状态
	bool isEnd;//是否为终结状态
	set<state*> epTrans;	//----------------ε能到达的状态集
	state() {
		stateID = -1;
		trans.clear();
		isEnd = false;
		isBegin = false;
	}
	/*state(int id, vector<edge> t, bool isB, bool isE) {
		stateID = id;
		trans.clear();
		for (int i = 0; i < t.size(); i++)
			trans.push_back(t[i]);
		isBegin = isB;
		isEnd = isE;
	}*/
};//状态

typedef struct FAMachine {
	vector<state*> states; //全部的状态
	state* beginState; //唯一的开始状态
	vector<state*> endState; //终结状态集合
	set<char> terminator;	//---------------- 终结符集
	FAMachine() { //构造函数
		beginState = NULL;
	}
	/* x:单字符自动机构造 */
	FAMachine(char ch) {
		state* begin = new state;
		state* end = new state;
		end->isEnd = true;
		begin->isBegin = true;
		begin->stateID = 0;
		end->stateID = 1;
		begin->trans.push_back(edge(ch, end->stateID));
		beginState = begin;
		endState.push_back(end);
		states.push_back(begin);
		states.push_back(end);
	}

	//添加终结符集
	void initTerminator(string str) {
		for (int i = 0; i < str.length(); i++) {
			if (str[i] >= 'a' && str[i] <= 'z') {
				terminator.insert(str[i]);
			}
		}
	}

}FAMachine;//自动机

//===================================生成NFA=====================================

class FAM_Manager {
public:

	/* x|y */
	FAMachine ORoperation(FAMachine x, FAMachine y) {//或运算
		FAMachine res = FAMachine();
		res.beginState = new state;
		res.beginState->stateID = 0;
		res.beginState->isBegin = true;
		res.states.push_back(res.beginState);
		for (int i = 0; i < x.states.size(); i++) {
			x.states[i]->stateID += 1;
			x.states[i]->isBegin = false;
			for (int j = 0; j < x.states[i]->trans.size(); j++)
				x.states[i]->trans[j].nex += 1;
			res.states.push_back(x.states[i]);
		}
		for (int i = 0; i < y.states.size(); i++) {
			y.states[i]->stateID += x.states.size() + 1;
			y.states[i]->isBegin = false;
			for (int j = 0; j < y.states[i]->trans.size(); j++)
				y.states[i]->trans[j].nex += x.states.size() + 1;
			res.states.push_back(y.states[i]);
		}
		res.beginState->trans.push_back(edge(96, x.beginState->stateID));
		res.beginState->trans.push_back(edge(96, y.beginState->stateID));
		state* end = new state;
		end->isEnd = true;
		end->stateID = x.states.size() + y.states.size() + 1;
		for (int i = 0; i < x.endState.size(); i++) {
			x.endState[i]->trans.push_back(edge(96, end->stateID));
			x.endState[i]->isEnd = false;
		}
		for (int i = 0; i < y.endState.size(); i++) {
			y.endState[i]->trans.push_back(edge(96, end->stateID));
			y.endState[i]->isEnd = false;
		}
		res.endState.push_back(end);
		res.states.push_back(end);
		return res;
	}
	/* x·y */
	FAMachine joinOperation(FAMachine x, FAMachine y) {//连接(乘积)运算
		FAMachine res;
		res.beginState = x.beginState;
		for (int i = 0; i < x.states.size(); i++) {
			res.states.push_back(x.states[i]);
		}
		for (int i = 0; i < y.states.size(); i++) {
			y.states[i]->isBegin = false;
			y.states[i]->stateID += x.states.size();
			for (int j = 0; j < y.states[i]->trans.size(); j++) {
				y.states[i]->trans[j].nex += x.states.size();
			}
			res.states.push_back(y.states[i]);
		}
		for (int i = 0; i < x.endState.size(); i++) {
			x.endState[i]->isEnd = false;
			x.endState[i]->trans.push_back(edge(96, y.beginState->stateID));
		}
		for (int i = 0; i < y.endState.size(); i++) {
			res.endState.push_back(y.endState[i]);
		}
		return res;
	}
	/* x* */
	FAMachine closureOperation(FAMachine x) {//闭包运算
		FAMachine res;
		state* newstate = new state;
		newstate->stateID = 0;
		newstate->isBegin = true;
		newstate->isEnd = true;
		res.states.push_back(newstate);
		for (int i = 0; i < x.states.size(); i++) {
			x.states[i]->isBegin = false;
			x.states[i]->stateID += 1;
			for (int j = 0; j < x.states[i]->trans.size(); j++) {
				x.states[i]->trans[j].nex += 1;
			}
			res.states.push_back(x.states[i]);
		}
		newstate->trans.push_back(edge(96, x.beginState->stateID));
		res.beginState = newstate;
		for (int i = 0; i < x.endState.size(); i++) {
			x.endState[i]->isEnd = false;
			x.endState[i]->trans.push_back(edge(96, res.beginState->stateID));
		}
		res.endState.push_back(newstate);
		return res;
	}
	/* x+ (= x·x* ) */

	void display(FAMachine fam)
	{
		cout << "共有" << fam.states.size() << "个状态，初态为" << fam.beginState->stateID << endl;
		cout << "有穷字母表为：{";
		for (auto i : fam.terminator)
		{
			cout << i << " ";
		}
		cout << "}" << endl;

		cout << "终态集为：{";
		for (int i = 0; i < fam.endState.size(); i++)
		{
			cout << fam.endState[i]->stateID << " ";
		}
		cout << "}" << endl;

		cout << "转移函数为：" << endl;
		for (int i = 0; i < fam.states.size(); i++)
		{
			for (int j = 0; j < fam.states[i]->trans.size(); j++)
			{
				if (fam.states[i]->isEnd)//起点是终态
					cout << "<" << fam.states[i]->stateID << ">";
				else
					cout << fam.states[i]->stateID;
				cout << "-->";
				if (fam.states[i]->trans[j].isEmpty())
					cout << "ε";
				else
					cout << fam.states[i]->trans[j].c;

				int nexNum = fam.states[i]->trans[j].nex;
				if (fam.states[nexNum]->isEnd)//下个状态是终态
				{
					cout << "--><" << fam.states[nexNum]->stateID << ">\t";
				}
				else
				{
					cout << "-->" << fam.states[nexNum]->stateID << "\t";
				}
			}
			cout << endl;
		}
	}

};

class NFAgeneration {
private:
	FAM_Manager manager;
	string re;
	string post = "";
	FAMachine nfa;
public:
	FAMachine& getnfa();
	string getStr();
	void input();
	bool check();
	void add_join_symbol();
	int priority(char ch);
	void regex2post();
	void post2nfa();
};

//获取nfa
FAMachine& NFAgeneration::getnfa() {
	return nfa;
}

//获取正则表达式
string NFAgeneration::getStr() {
	return re;
}

/* 输入正则表达式 */
void NFAgeneration::input() {
	cout << "请输入正则表达式：" << endl;
	do {
		cin >> re;
	} while (!check());
}

/* 检查正则表达式的输入是否正确，不正确则返回false */
bool NFAgeneration::check() {
	for (int i = 0; i < re.size(); i++) {
		char now = re.at(i);
		if (now >= 'a' && now <= 'z')
			;
		else if (now == '(' || now == ')' || now == '*' || now == '+' || now == '|')
			;
		else {
			cout << "有不合法字符！请重新输入：" << endl;
			return false;
		}
	}
	stack<char> paren;
	bool flag = 1;
	for (int i = 0; i < re.size(); i++) {
		if (re[i] == '(')
			paren.push(re[i]);
		else if (re[i] == ')')
			if (paren.empty())
				flag = 0;
			else
				paren.pop();
	}
	if (paren.size())
		flag = 0;
	if (!flag) {
		cout << "括号不匹配！请重新输入：" << endl;
		return false;
	}
	return true;
}

/* 用“&”表示连接操作的运算符，方便进行计算 */
void NFAgeneration::add_join_symbol() {
	string tmp = "";
	char now, nex;
	tmp += re.at(0);
	for (int i = 0; i < re.size() - 1; i++) {
		now = re[i];
		nex = re[i + 1];
		//在 ab a( 之间加上连接运算符
		if ((now >= 'a' && now <= 'z') || now == '*' || now == '+') {
			if (nex == '(' || (nex >= 'a' && nex <= 'z'))
				tmp += '&';
		}
		//在 *b +b )b 之间加上连接运算符
		else if (now == '*' || now == '+' || now == ')') {
			if (nex >= 'a' && nex <= 'z')
				tmp += '&';
		}
		tmp += nex;
	}
	tmp += '\0';
	re = tmp;
}

/* 返回运算符的优先级 */
int NFAgeneration::priority(char ch) {
	switch (ch) {
	case '*':
		return 4;
	case '+':
		return 3;
	case '&':
		return 2;
	case '|':
		return 1;
	case '(':
		return 0;
	default:
		return -1;
	}
}

/* 将中缀表达式转换为后缀形式 */
void NFAgeneration::regex2post() {
	add_join_symbol();
	stack<char> optr;//运算符栈
	for (int i = 0; i < re.size(); i++) {
		char now = re.at(i);
		if (now == '\0')
			break;
		if (now >= 'a' && now <= 'z') //字符直接累加
			post += now;
		else if (now == '(') //左括号入栈
			optr.push(now);
		else if (now == ')') {
			while (optr.top() != '(') { //栈顶元素不是左括号就累加并出栈
				post += optr.top();
				optr.pop();
			}
			optr.pop();//左括号出栈
		}
		else if (now == '*' || now == '+')
			post += now;
		else {// 运算符: | &
			while (!optr.empty()) {
				//将优先级大于等于的运算符出栈
				if (priority(optr.top()) >= priority(now)) {
					post += optr.top();
					optr.pop();
				}
				else
					break;
			}
			optr.push(now);//运算符入栈
		}
	}
	while (optr.size()) { //栈不为空
		post += optr.top();
		optr.pop();
	}
	cout << "后缀表达式: " << post << endl << endl;
}

void NFAgeneration::post2nfa() {
	nfa.initTerminator(re);

	stack<FAMachine> opnd;//操作符栈
	for (int i = 0; i < post.size(); i++) {
		char now = post.at(i);
		if (now >= 'a' && now <= 'z')
			opnd.push(FAMachine(now));
		else if (now == '*') { //闭包运算
			FAMachine tmp = opnd.top();
			opnd.pop();
			opnd.push(manager.closureOperation(tmp));
		}
		//else if (now == '+') { //正闭包运算
		//	FAMachine tmp, tmp2;
		//	tmp = opnd.top();
		//	opnd.pop();
		//	if (opnd.size())
		//		tmp2 = manager.joinOperation(opnd.top(), tmp);
		//	else
		//		tmp2 = manager.closureOperation(tmp);
		//	opnd.push(manager.closureOperation(tmp2, tmp));
		//}
		else if (now == '&') { //连接运算
			FAMachine fir = opnd.top();
			opnd.pop();
			FAMachine sed = opnd.top();
			opnd.pop();
			opnd.push(manager.joinOperation(sed, fir));
		}
		else if (now == '|') { //或运算
			FAMachine fir = opnd.top();
			opnd.pop();
			FAMachine sed = opnd.top();
			opnd.pop();
			opnd.push(manager.ORoperation(sed, fir));
		}
	}
	if (opnd.size()) {
		nfa = opnd.top();
		opnd.pop();
	}
}

//=================================NFA 2 DFA=====================================

class NFA2DFA {
private:
	FAMachine dfa;	//注意dfa的state下标和stateID是对应的
	state startState;//起始状态
public:
	FAMachine getdfa();
	void initEpTrans(FAMachine& nfa);//初始化nfa的ε集合
	bool IsEnd(FAMachine& nfa, set<state*> s);//判断是不是终态集
	set<state*> epcloure(set<state*> s, FAMachine nfa);//求ε闭包
	set<state*> moveEpclourse(set<state*> s, char& ch, FAMachine nfa);//添加字符ch对应的状态集
	void nfa2dfa(FAMachine& nfa, string str);
};

FAMachine NFA2DFA::getdfa() {
	return dfa;
}

//添加每个点的terminator终态集
void NFA2DFA::initEpTrans(FAMachine& nfa) {
	for (int i = 0; i < nfa.states.size(); i++) {//遍历所有状态集
		for (int j = 0; j < nfa.states[i]->trans.size(); j++) {//遍历当前状态节点连接的所有边
			if (nfa.states[i]->trans[j].isEmpty()) {//如果边权为空
				nfa.states[i]->epTrans.insert(nfa.states[nfa.states[i]->trans[j].nex]);//给states[i]插入由ε可达的点states[e.nex]
			}
		}
	}
}

//求状态集合s的闭包
set<state*> NFA2DFA::epcloure(set<state*> s, FAMachine nfa)
{
	stack<state*> epStack;
	for (auto i : s) {
		epStack.push(i);
	}
	while (!epStack.empty()) {
		state* sta = epStack.top();
		epStack.pop();
		for (auto i : sta->epTrans) {
			if (!s.count(i))//如果不在已有集合中
			{
				s.insert(i);
				epStack.push(i);
			}
		}
	}
	return s;
}

//判断状态集合s中是否有终结状态
bool NFA2DFA::IsEnd(FAMachine& nfa, set<state*> s) {
	for (auto i : s) {
		if (i->isEnd)
			return true;
	}
	return false;
}

//添加边ch对应的状态集
set<state*> NFA2DFA::moveEpclourse(set<state*> s, char& ch, FAMachine nfa)
{
	set<state*> stmp;
	for (auto i : s) {
		for (int j = 0; j < i->trans.size(); j++) {//这里应该遍历nfa中状态所连接的边
			if (i->trans[j].c == ch) {//找到nfa中对应的点
				stmp.insert(nfa.states[i->trans[j].nex]);//通过编号插入nfa状态
			}
		}
	}
	return stmp;
}

void NFA2DFA::nfa2dfa(FAMachine& nfa, string str)
{
	//初始化dfa起点
	startState.stateID = 0;
	startState.isBegin = true;
	dfa.beginState = &startState;
	dfa.states.push_back(&startState);

	dfa.initTerminator(str);

	set<state*> tmpSet; //计算闭包
	tmpSet.insert(nfa.beginState); //从nfa的初态开始遍历
	startState.epTrans = epcloure(tmpSet, nfa);
	startState.isEnd = IsEnd(nfa, startState.epTrans); //判断是否为终态

	set<set<state*> > SetInSet;//集合的集合
	queue<state*> q;
	q.push(&startState);
	while (!q.empty()) {
		state* tmpState = q.front();//dfa中的状态
		q.pop();
		for (auto i : dfa.terminator) { //遍历终结符集
			set<state*> stmp = moveEpclourse(tmpState->epTrans, i, nfa);//边i能到达的状态集合
			stmp = epcloure(stmp, nfa);
			if (!SetInSet.count(stmp) && !stmp.empty()) { //如果边能到达的状态集合未出现过
				SetInSet.insert(stmp);
				state* S = new state(); //建点
				S->epTrans = stmp;
				S->stateID = dfa.states.size();

				tmpState->trans.push_back(edge(i, dfa.states.size()));//建边，存到状态节点中

				S->isEnd = IsEnd(nfa, S->epTrans);//S是否为终结符

				q.push(S);
				dfa.states.push_back(S);
			}
			else {
				for (int j = 0; j < dfa.states.size(); j++) {
					if (stmp == dfa.states[j]->epTrans) {//找到dfa中连接着stmp的那个状态
						tmpState->trans.push_back(edge(i, j));
						break;
					}
				}
			}
		}
	}
	for (int i = 0; i < dfa.states.size(); i++) { //放入终态集合
		if (dfa.states[i]->isEnd) {
			dfa.endState.push_back(dfa.states[i]);
		}
	}
}

//==================================DFA最小化====================================

const int  Maxn = 100;
class DFAminimize {
	vector<pair<int, int> > vlist[Maxn][Maxn];
	bool mark[Maxn][Maxn];
	int g[Maxn][Maxn];
	bool END[Maxn];
	map<char, int> mofc;//字符集对0-m的映射
	map<int, char> mofi;

public:
	void del(int u, int v);
	void output(int m, int n, FAMachine);
	void tminDFA(FAMachine dfa);

};

void DFAminimize::del(int u, int v)
{
	int i, j, k;
	mark[u][v] = true;
	for (i = 0; i < vlist[u][v].size(); i++)
	{
		j = vlist[u][v][i].first;
		k = vlist[u][v][i].second;
		if (!mark[j][k])
			del(j, k);
	}
	vlist[u][v].clear();
}

void DFAminimize::output(int m, int n, FAMachine dfa) {
	int flag[1000];
	memset(flag, -1, sizeof(flag));
	for (int i = 1; i < n; i++) {
		for (int j = 0; j < i; j++) {
			if (flag[j] == -1) {
				if (!mark[i][j]) {
					flag[i] = j;
					for (int k = 0; k < m; k++) {
						if (g[j][k] != -1)
							g[j][k] = j;
					}
				}
			}
		}
	}
	for (int i = 0; i < n; i++) {
		for (int j = 0; j < m; j++) {
			if (flag[i] == -1 && g[i][j] != -1) {
				int x;
				if (flag[g[i][j]] != -1) {
					x = flag[g[i][j]];
				}
				else {
					x = g[i][j];
				}
				bool endi = false, endx = false;
				for (auto it : dfa.endState)
				{
					if (i == it->stateID && it->isEnd)
						endi = true;
					if (x == it->stateID && it->isEnd)
						endx = true;
				}
				if (endi)
					cout << '<' << i << '>';
				else
					cout << i;
				cout << "-->" << mofi[j] << "-->";
				if (endx)
					cout << '<' << x << '>' << endl;
				else
					cout << x << endl;
			}
		}
	}
}

void DFAminimize::tminDFA(FAMachine dfa)
{
	int n, m;
	int M[1000];
	memset(M, 0, sizeof(M));
	n = dfa.states.size();
	m = dfa.terminator.size();

	memset(g, -1, sizeof(g));

	int i = 0;
	for (set<char>::iterator it = dfa.terminator.begin(); it != dfa.terminator.end(); it++)
	{
		mofc[*it] = i;
		mofi[i] = *it;
		i++;
	}

	for (auto it : dfa.states)
	{
		int u = it->stateID;
		for (auto itt : it->trans)
		{
			char w = itt.c;
			int v = itt.nex;
			g[u][mofc[w]] = v;
			M[u]++;
		}
	}

	int nendstateNum = dfa.states.size() - dfa.endState.size();

	memset(END, false, sizeof(END));

	for (auto it : dfa.states)
	{
		if (it->isEnd)
			END[it->stateID] = true;
	}

	memset(mark, false, sizeof(mark));

	for (int i = 0; i <= n; i++)
		for (int j = 0; j <= n; j++)
			vlist[i][j].clear();
	for (int i = 0; i < n; i++)
		for (int j = 0; j < i; j++)
			if (END[i] != END[j])
				mark[i][j] = true;

	bool flag;
	for (int j = 0; j < n - 1; j++)
		for (int i = j + 1; i < n; i++)
			if (END[i] == END[j])
			{
				flag = false;
				if (M[j] == 0 && M[i] != 0) {
					for (int p = 0; p < m; p++) {
						if (g[i][p] != -1) {
							int v = g[i][p];
							if (mark[i][v] || mark[v][i]) {
								del(i, j);
								flag = true;
								break;
							}
						}
					}
				}
				if (M[i] == 0 && M[j] != 0) {
					for (int p = 0; p < m; p++) {
						if (g[j][p] != -1) {
							int v = g[j][p];
							if (mark[j][v] || mark[v][j]) {
								del(i, j);
								flag = true;
								break;
							}
						}
					}
				}
				for (int k = 0; k < m; k++)
				{
					if (g[i][k] != -1) {
						for (int p = 0; p < m; p++) {
							if (g[j][p] != -1) {
								int u = g[i][k];
								int v = g[j][p];
								if (mark[i][u] || mark[u][i] || mark[u][v] || mark[v][u])
								{
									flag = true;
									if (!mark[i][j])
										del(i, j);
								}

								if (flag)
									break;
							}
						}
					}
				}
				if (!flag)
					for (int k = 0; k < m; k++)
					{
						if (g[i][k] != -1) {
							for (int p = 0; p < m; p++) {
								if (g[j][p] != -1)
								{
									int u = g[i][k];
									int v = g[j][p];
									if (u != v && (min(u, v) != j || max(u, v) != i))
										vlist[max(u, v)][min(u, v)].push_back(make_pair(i, j));
								}
							}
						}
					}
			}
	output(m, n, dfa);
}

int main() {
	FAM_Manager manager;

	while (true) {
		cout << "============================生成nfa=============================" << endl;
		NFAgeneration nfaGeneration;
		nfaGeneration.input();
		nfaGeneration.regex2post();
		nfaGeneration.post2nfa();
		manager.display(nfaGeneration.getnfa());

		cout << "============================nfa转dfa============================" << endl;
		NFA2DFA nfa2dfa;
		nfaGeneration.getnfa();
		nfa2dfa.initEpTrans(nfaGeneration.getnfa());
		nfa2dfa.nfa2dfa(nfaGeneration.getnfa(), nfaGeneration.getStr());
		manager.display(nfa2dfa.getdfa());

		cout << "============================dfa最小化============================" << endl;
		DFAminimize dfaMini;
		dfaMini.tminDFA(nfa2dfa.getdfa());
	}
	return 0;
}