#ifndef _EXPRESSIONS_HEADER
#define _EXPRESSIONS_HEADER

#include <string>
#include <stdio.h>
#include <vector>

#include "Compiler.h"
#include "Value.h"

namespace Jet
{
	class Compiler;

	class Expression
	{
	public:

		Expression()
		{
			Parent = 0;
		}

		virtual ~Expression()
		{

		}

		Expression* Parent;
		virtual void SetParent(Expression* parent)
		{
			this->Parent = parent;
		}

		virtual void Compile(CompilerContext* context) = 0;
	};

	class IStorableExpression
	{
	public:
		virtual void CompileStore(CompilerContext* context) = 0;
	};

	class NameExpression: public Expression, public IStorableExpression
	{
		std::string name;
	public:
		NameExpression(std::string name)
		{
			this->name = name;
		}

		std::string GetName()
		{
			return this->name;
		}

		void Compile(CompilerContext* context);

		void CompileStore(CompilerContext* context)
		{
#if FORCE_USING_GLOBAL
			if (context->IsLocal(name))
			{
				context->Store(name);
			}
			else if(context->IsGlobal(name))
			{
				context->Store(name);
			}
			else
			{
				context->RegisterLocal(name);
				context->Store(name);
			}
#else
			context->Store(name);
#endif			
		}
	};

	class ArrayExpression: public Expression
	{
		std::vector<Expression*> initializers;
	public:
		ArrayExpression(std::vector<Expression*>&& inits) : initializers(inits)
		{

		}

		~ArrayExpression()
		{
			//if (this->initializers)
			//{
			for (auto ii: this->initializers)
				delete ii;
			//}
			//delete this->initializers;
		}

		virtual void SetParent(Expression* parent)
		{
			this->Parent = parent;
			//if (this->initializers)
			//{
			for (auto ii: this->initializers)
				ii->SetParent(this);
			//}
		}

		void Compile(CompilerContext* context);
	};

	class ObjectExpression: public Expression
	{
		std::vector<std::pair<std::string, Expression*>>* inits;
	public:
		ObjectExpression()
		{
			inits = 0;
		}

		ObjectExpression(std::vector<std::pair<std::string, Expression*>>* initializers) : inits(initializers)
		{
		}

		~ObjectExpression()
		{
			if (this->inits)
				for (auto ii: *this->inits)
					delete ii.second;
			delete this->inits;
		}

		virtual void SetParent(Expression* parent)
		{
			this->Parent = parent;
			if (this->inits)
				for (auto ii: *this->inits)
					ii.second->SetParent(this);
		}

		void Compile(CompilerContext* context);
	};

	/// <summary>
	/// 变量定义
	/// </summary>
	struct VarDefine 
	{
		/// <summary>
		/// 变量名
		/// </summary>
		Token		m_Name;

		/// <summary>
		/// 变量初始化表达式
		/// </summary>
		Expression*	m_Experssion = nullptr;
	};

	class LocalExpression: public Expression
	{
		std::vector<VarDefine>*	defines = nullptr;
	public:
		LocalExpression(std::vector<VarDefine>* _defines)
		{
			defines = _defines;
		}

		~LocalExpression()
		{
			for (auto d : *this->defines)
			{
				if (d.m_Experssion != nullptr)
				{
					delete d.m_Experssion;
				}
			}
			delete this->defines;
		}

		virtual void SetParent(Expression* parent)
		{
			this->Parent = parent;
			for (auto d : *this->defines)
			{
				if (d.m_Experssion != nullptr)
				{
					d.m_Experssion->SetParent(this);
				}
			}
		}

		void Compile(CompilerContext* context);
	};

	class GlobalExpression : public Expression
	{
		std::vector<VarDefine>*	defines = nullptr;
	public:
		GlobalExpression(std::vector<VarDefine>* _defines)
		{
			defines = _defines;
		}

		~GlobalExpression()
		{
			for (auto d : *this->defines)
			{
				if (d.m_Experssion != nullptr)
				{
					delete d.m_Experssion;
				}
			}
			delete this->defines;
		}

		virtual void SetParent(Expression* parent)
		{
			this->Parent = parent;
			for (auto d : *this->defines)
			{
				if (d.m_Experssion != nullptr)
				{
					d.m_Experssion->SetParent(this);
				}
			}
		}

		void Compile(CompilerContext* context);
	};

	class IntNumberExpression : public Expression
	{
		int64_t value;
	public:
		IntNumberExpression(int64_t value)
		{
			this->value = value;
		}

		int64_t GetValue()
		{
			return this->value;
		}

		void Compile(CompilerContext* context);
	};

	class RealNumberExpression: public Expression
	{
		double value;
	public:
		RealNumberExpression(double value)
		{
			this->value = value;
		}

		double GetValue()
		{
			return this->value;
		}

		void Compile(CompilerContext* context);
	};

	class NullExpression: public Expression
	{
	public:
		NullExpression()
		{
		}

		void print()
		{
			printf("Null");
		}

		void Compile(CompilerContext* context);
	};

	class StringExpression: public Expression
	{
		std::string value;
	public:
		StringExpression(std::string value)
		{
			this->value = value;
		}

		std::string GetValue()
		{
			return this->value;
		}

		void Compile(CompilerContext* context);
	};

	class IndexExpression: public Expression, public IStorableExpression
	{
		Expression*index;
	public:
		Expression* left;
		Token token;
		IndexExpression(Expression* left, Expression* index, Token t)
		{
			this->token = t;
			this->left = left;
			this->index = index;
		}

		~IndexExpression()
		{
			delete left;
			delete index;
		}

		void Compile(CompilerContext* context);

		void CompileStore(CompilerContext* context);
	};

	class AssignExpression: public Expression
	{
		Expression* left;
		Expression* right;
	public:
		AssignExpression(Expression* l, Expression* r)
		{
			this->left = l;
			this->right = r;
		}

		~AssignExpression()
		{
			delete this->right;
			delete this->left;
		}

		virtual void SetParent(Expression* parent)
		{
			this->Parent = parent;
			right->SetParent(this);
			left->SetParent(this);
		}

		void Compile(CompilerContext* context);
	};

	class OperatorAssignExpression: public Expression
	{
		Token token;

		Expression* left;
		Expression* right;
	public:
		OperatorAssignExpression(Token token, Expression* l, Expression* r)
		{
			this->token = token;
			this->left = l;
			this->right = r;
		}

		~OperatorAssignExpression()
		{
			delete this->right;
			delete this->left;
		}

		void SetParent(Expression* parent)
		{
			this->Parent = parent;
			right->SetParent(this);
			left->SetParent(this);
		}

		void Compile(CompilerContext* context);
	};

	class SwapExpression: public Expression
	{
		Expression* left;
		Expression* right;
	public:
		SwapExpression(Expression* l, Expression* r)
		{
			this->left = l;
			this->right = r;
		}

		~SwapExpression()
		{
			delete this->right;
			delete this->left;
		}

		void SetParent(Expression* parent)
		{
			this->Parent = parent;
			right->SetParent(this);
			left->SetParent(this);
		}

		void Compile(CompilerContext* context);
	};

	class PrefixExpression: public Expression
	{
		Token _operator;

		Expression* right;
	public:
		PrefixExpression(Token type, Expression* r)
		{
			this->_operator = type;
			this->right = r;
		}

		~PrefixExpression()
		{
			delete this->right;
		}

		void SetParent(Expression* parent)
		{
			this->Parent = parent;
			right->SetParent(this);
		}

		void Compile(CompilerContext* context);
	};

	class PostfixExpression: public Expression
	{
		Token _operator;

		Expression* left;
	public:
		PostfixExpression(Expression* l, Token type)
		{
			this->_operator = type;
			this->left = l;
		}

		~PostfixExpression()
		{
			delete this->left;
		}

		void SetParent(Expression* parent)
		{
			this->Parent = parent;
			left->SetParent(this);
		}

		void Compile(CompilerContext* context);
	};

	class OperatorExpression: public Expression
	{
		Token _operator;

		Expression* left, *right;

	public:
		OperatorExpression(Expression* l, Token type, Expression* r)
		{
			this->_operator = type;
			this->left = l;
			this->right = r;
		}

		~OperatorExpression()
		{
			delete this->right;
			delete this->left;
		}

		void SetParent(Expression* parent)
		{
			this->Parent = parent;
			left->SetParent(this);
			right->SetParent(this);
		}

		void Compile(CompilerContext* context);
	};

	class StatementExpression: public Expression
	{
	public:
	};

	class BlockExpression: public Expression
	{

	public:
		std::vector<Expression*> statements;
		BlockExpression(Token token, std::vector<Expression*>&& statements) : statements(statements)
		{

		}

		BlockExpression(std::vector<Expression*>&& statements) : statements(statements)
		{

		}

		~BlockExpression()
		{
			for (auto ii: this->statements)
				delete ii;
		}

		BlockExpression() { };

		void SetParent(Expression* parent)
		{
			this->Parent = parent;
			for (auto ii: statements)
				ii->SetParent(this);
		}

		void Compile(CompilerContext* context)
		{
			for (auto ii: statements)
				ii->Compile(context);
		}
	};

	class ScopeExpression: public BlockExpression
	{

	public:
		//add a list of local variables here mayhaps?

		ScopeExpression(BlockExpression* r)
		{
			this->statements = r->statements;
			r->statements.clear();
			delete r;
		}

		void Compile(CompilerContext* context)
		{
			//push scope
			context->PushScope();

			BlockExpression::Compile(context);

			//pop scope
			context->PopScope();
		}
	};

	class WhileExpression: public Expression
	{
		Expression* condition;
		ScopeExpression* block;
		Token token;
	public:

		WhileExpression(Token token, Expression* cond, ScopeExpression* block)
		{
			this->condition = cond;
			this->block = block;
			this->token = token;
		}

		~WhileExpression()
		{
			delete condition;
			delete block;
		}

		virtual void SetParent(Expression* parent)
		{
			this->Parent = parent;
			block->SetParent(this);
			condition->SetParent(this);
		}

		void Compile(CompilerContext* context)
		{
			context->Line(token.line);

			std::string uuid = context->GetUUID();
			context->Label("loopstart_"+uuid);
			this->condition->Compile(context);
			context->JumpFalse(("loopend_"+uuid).c_str());

			context->PushLoop("loopend_"+uuid, "loopstart_"+uuid);
			this->block->Compile(context);
			context->PopLoop();

			context->Jump(("loopstart_"+uuid).c_str());
			context->Label("loopend_"+uuid);
		}
	};

	class ForExpression: public Expression
	{
		Expression* condition, *initial, *incr;
		ScopeExpression* block;
		Token token;
	public:
		ForExpression(Token token, Expression* init, Expression* cond, Expression* incr, ScopeExpression* block)
		{
			this->condition = cond;
			this->block = block;
			this->incr = incr;
			this->initial = init;
			this->token = token;
		}

		~ForExpression()
		{
			delete this->condition;
			delete this->block;
			delete this->incr;
			delete this->initial;
		}

		void SetParent(Expression* parent)
		{
			this->Parent = parent;
			block->SetParent(this);
			incr->SetParent(block);//block);
			condition->SetParent(this);//block);
			initial->SetParent(block);
		}

		void Compile(CompilerContext* context)
		{
			context->Line(token.line);

			std::string uuid = context->GetUUID();
			this->initial->Compile(context);
			context->Label("forloopstart_"+uuid);
			this->condition->Compile(context);
			context->JumpFalse(("forloopend_"+uuid).c_str());

			context->PushLoop("forloopend_"+uuid, "forloopcontinue_"+uuid);
			this->block->Compile(context);
			context->PopLoop();

			//this wont work if we do some kind of continue keyword unless it jumps to here
			context->Label("forloopcontinue_"+uuid);
			this->incr->Compile(context);
			context->Jump(("forloopstart_"+uuid).c_str());
			context->Label("forloopend_"+uuid);
		}
	};

	class ForEachExpression: public Expression
	{
		Token name;
		Expression* container;
		ScopeExpression* block;
	public:
		ForEachExpression(Token name, Expression* container, ScopeExpression* block)
		{
			this->container = container;
			this->block = block;
			this->name = name;
		}

		~ForEachExpression()
		{
			delete this->block;
			delete this->container;
		}

		void SetParent(Expression* parent)
		{
			this->Parent = parent;
			block->SetParent(this);
		}

		void Compile(CompilerContext* context)
		{
			context->PushScope();

			auto uuid = context->GetUUID();
			context->RegisterLocal(this->name.text);
			context->RegisterLocal("_iter");

			//context->Load(this->container.text);
			this->container->Compile(context);
			context->Duplicate();
			context->LoadIndex("iterator");
			context->ECall(1);
			//context->Duplicate();
			context->Store("_iter");
			//context->JumpFalse(("_foreachend"+uuid).c_str());

			context->Label("_foreachstart"+uuid);

			context->Load("_iter");
			context->Duplicate();
			context->LoadIndex("advance");
			context->ECall(1);
			context->JumpFalse(("_foreachend"+uuid).c_str());

			context->Load("_iter");
			context->Duplicate();
			context->LoadIndex("current");
			context->ECall(1);
			context->Store(this->name.text);

			//context->ForEach(this->name.text, "_foreachstart"+uuid, "_foreachend"+uuid);
			//finish implementing foreach instructions
			context->PushLoop("_foreachend"+uuid, "_foreachstart"+uuid);
			this->block->Compile(context);
			context->PopLoop();

			//context->PostForEach(

			

			context->Jump(("_foreachstart"+uuid).c_str());
			context->Label("_foreachend"+uuid);

			context->PopScope();
		}
	};


	struct Branch
	{
		BlockExpression* block;
		Expression* condition;

		Branch(BlockExpression* block, Expression* condition)
		{
			this->block = block;
			this->condition = condition;
		}

		Branch(Branch&& other)
		{
			this->block = other.block;
			this->condition = other.condition;
			other.block = 0;
			other.condition = 0;
		}
		~Branch()
		{
			delete condition;
			delete block;
		}
	};
	class IfExpression: public Expression
	{
		std::vector<Branch*> branches;
		Branch* Else;
		Token token;
	public:
		IfExpression(Token token, std::vector<Branch*>&& branches, Branch* elseBranch)
		{
			this->branches = branches;
			this->Else = elseBranch;
			this->token = token;
		}

		~IfExpression()
		{
			delete Else;
			for (auto ii : this->branches)
				delete ii;
			//delete this->branches;
		}

		virtual void SetParent(Expression* parent)
		{
			this->Parent = parent;
			if (this->Else)
				this->Else->block->SetParent(this);
			for (auto& ii: branches)
			{
				ii->block->SetParent(this);
				ii->condition->SetParent(this);
			}
		}

		void Compile(CompilerContext* context)
		{
			context->Line(token.line);

			std::string uuid = context->GetUUID();
			std::string bname = "ifstatement_" + uuid + "_I";
			int pos = 0;
			bool hasElse = this->Else ? this->Else->block->statements.size() > 0 : false;
			for (auto& ii: this->branches)
			{
				if (pos != 0)//no jump label needed on first one
					context->Label(bname);

				ii->condition->Compile(context);

				//if no else and is last go to end
				if (hasElse == false && pos == (this->branches.size()-1))
					context->JumpFalse(("ifstatementend_"+uuid).c_str());
				else
					context->JumpFalse((bname+"I").c_str());

				ii->block->Compile(context);

				if (pos != (this->branches.size()-1) || hasElse)//if isnt last one
					context->Jump(("ifstatementend_"+uuid).c_str());

				bname += "I";
				pos++;
			}

			if (hasElse)//this->Else && this->Else->block->statements->size() > 0)
			{
				context->Label(bname);
				this->Else->block->Compile(context);
			}
			context->Label("ifstatementend_"+uuid);
		}
	};

	class CallExpression: public Expression
	{
		Token token;
		Expression* left;
		std::vector<Expression*>* args;
	public:
		friend class FunctionParselet;
		CallExpression(Token token, Expression* left, std::vector<Expression*>* args)
		{
			this->token = token;
			this->left = left;
			this->args = args;
		}

		~CallExpression()
		{
			delete this->left;
			if (args)
			{
				for (auto ii: *args)
					delete ii;

				delete args;
			}
		}

		void SetParent(Expression* parent)
		{
			this->Parent = parent;
			left->SetParent(this);
			for (auto ii: *args)
				ii->SetParent(this);
		}

		void Compile(CompilerContext* context);
	};

	class FunctionExpression: public Expression
	{
		Expression* name;
		std::vector<Expression*>* args;
		ScopeExpression* block;
		Token token;
		NameExpression* varargs;
		bool	isMethod = false;		// is class method
	public:

		FunctionExpression(Token token, Expression* name, std::vector<Expression*>* args, ScopeExpression* block, NameExpression* varargs = 0,bool method=false)
		{
			this->args = args;
			this->block = block;
			this->name = name;
			this->token = token;
			this->varargs = varargs;
			this->isMethod = method;
		}

		~FunctionExpression()
		{
			delete block;
			delete name;
			delete varargs;

			if (args)
			{
				for (auto ii: *args)
					delete ii;
				delete args;
			}
		}

		void SetParent(Expression* parent)
		{
			this->Parent = parent;
			block->SetParent(this);
			if (name)
				name->SetParent(this);
			for (auto ii: *args)
				ii->SetParent(this);
		}

		void Compile(CompilerContext* context);
	};

	class ReturnExpression: public Expression
	{
		Token token;
		Expression* right;
	public:
		ReturnExpression(Token token, Expression* right)
		{
			this->token = token;
			this->right = right;
		}

		~ReturnExpression()
		{
			delete this->right;
		}

		void SetParent(Expression* parent)
		{
			this->Parent = parent;
			if (right)
				this->right->SetParent(this);
		}

		void Compile(CompilerContext* context)
		{
			context->Line(token.line);

			if (right)
				right->Compile(context);
			else
				context->Null();//bad value to prevent use

			context->Return();
		}
	};

	class BreakExpression: public Expression
	{
	public:

		void SetParent(Expression* parent)
		{
			this->Parent = parent;
		}

		void Compile(CompilerContext* context)
		{
			context->Break();
		}
	};

	class ContinueExpression: public Expression
	{
	public:
		void SetParent(Expression* parent)
		{
			this->Parent = parent;
		}

		void Compile(CompilerContext* context)
		{
			context->Continue();
		}
	};

	class YieldExpression: public Expression
	{
		Token token;
		Expression* right;
	public:
		YieldExpression(Token t, Expression* right)
		{
			this->token = t;
			this->right = right;
		}

		void SetParent(Expression* parent)
		{
			this->Parent = parent;
			if (right)
				right->SetParent(this);
		}

		void Compile(CompilerContext* context)
		{
			if (right)
				right->Compile(context);
			else
				context->Null();

			context->Yield();

			if (dynamic_cast<BlockExpression*>(this->Parent) !=0)
				context->Pop();
		}
	};

	class ResumeExpression: public Expression
	{
		Token token;
		Expression* right;
	public:
		ResumeExpression(Token t, Expression* right)
		{
			this->token = t;
			this->right = right;
		}

		void SetParent(Expression* parent)
		{
			this->Parent = parent;
			if (right)
				right->SetParent(this);
		}

		void Compile(CompilerContext* context)
		{
			this->right->Compile(context);

			context->Resume();

			if (dynamic_cast<BlockExpression*>(this->Parent))
				context->Pop();
		}
	};

	class ClassExpression : public Expression
	{
		std::string		m_Name;
		std::string		m_Base;
		std::map<std::string, FunctionExpression*>	m_Functions;
		std::vector<VarDefine>						m_Fields;
	public:
		ClassExpression(const std::string& name, const std::string& baseName, const std::map<std::string, FunctionExpression*>& funcs, const std::vector<VarDefine>& fields) :m_Name(name), m_Fields(fields), m_Functions(funcs), m_Base(baseName)
		{
		}

		~ClassExpression()
		{
			for (auto& i : m_Functions)
			{
				delete i.second;
			}
			m_Functions.clear();
			for (auto& i : m_Fields)
			{
				if (i.m_Experssion != nullptr)
				{
					delete i.m_Experssion;
				}
			}
			m_Fields.clear();
		}

		void SetParent(Expression* parent)
		{
			this->Parent = parent;
			for (auto& i:m_Functions)
			{
				i.second->SetParent(this);
			}

			for (auto& i : m_Fields)
			{
				if (i.m_Experssion != nullptr)
				{
					i.m_Experssion->SetParent(this);
				}
			}
		}

		void Compile(CompilerContext* context);
	};
}
#endif