/*
 * ROSTask.h
 *
 *  Created on: Nov 5, 2013
 *      Author: dan
 */

#ifndef ROSTASK_H_
#define ROSTASK_H_

#include "SynchCout.h"
#include "EventSystem.h"
#include "TaskResult.h"

#include <ros/ros.h>
#include <std_msgs/String.h>
#include <diagnostic_updater/diagnostic_updater.h>

#include <map>
class LocalTasks{
	typedef decision_making::TaskResult (* callTask)(std::string, const decision_making::FSMCallContext&, decision_making::EventQueue&);
	typedef std::map<std::string, callTask> callbacks;
	static callbacks& get(){ static callbacks t; return t; }
public:
	static void registrate(std::string task_name, callTask cb){
		get()[task_name]=cb;
	}
	static bool registrated(std::string task_name){
		return get().find(task_name)!=get().end();
	}
	static decision_making::TaskResult call(
			std::string task_name,
			std::string task_address,
			const decision_making::FSMCallContext& call_ctx,
			decision_making::EventQueue& events){
		return get()[task_name](task_address, call_ctx, events);
	}
};


decision_making::TaskResult callTask(std::string task_address, const decision_making::FSMCallContext& call_ctx, decision_making::EventQueue& events);
#define CALL_REMOTE(NAME, CALLS, EVENTS) boost::bind(&callTask, #NAME, CALLS, EVENTS)

#define DECISION_MAKING_EVENTS_MACROSES

#define ON_FUNCTION(FNAME) void FNAME(std::string name, std::string type, const decision_making::FSMCallContext& call_ctx, decision_making::EventQueue& events, decision_making::TaskResult result)
#define NO_RESULT decision_making::TaskResult::UNDEF()

ON_FUNCTION(on_fsm_start);
#define ON_FSM_START(NAME, CALLS, EVENTS) on_fsm_start(NAME, "FSM", CALLS, EVENTS, NO_RESULT)

ON_FUNCTION(on_fsm_end);
#define ON_FSM_END(NAME, CALLS, EVENTS, RESULT) on_fsm_end(NAME, "FSM", CALLS, EVENTS, RESULT)


ON_FUNCTION(on_fsm_state_start);
#define ON_FSM_STATE_START(NAME, CALLS, EVENTS) on_fsm_state_start(NAME, "FSM_STATE", decision_making::FSMCallContext(CALLS,NAME), EVENTS, NO_RESULT)
ON_FUNCTION(on_fsm_state_end);
#define ON_FSM_STATE_END(NAME, CALLS, EVENTS) on_fsm_state_end(NAME, "FSM_STATE", decision_making::FSMCallContext(CALLS,NAME), EVENTS, NO_RESULT)


ON_FUNCTION(on_bt_node_start);
#define ON_BT_NODE_START(NAME, TYPE, CALLS, EVENTS) on_bt_node_start(NAME, std::string("BT_")+TYPE, CALLS, EVENTS, NO_RESULT)
ON_FUNCTION(on_bt_node_end);
#define ON_BT_NODE_END(NAME, CALLS, EVENTS, RESULT) on_bt_node_end(NAME, "BT_NODE", CALLS, EVENTS, RESULT)


class RosConstraints{
public:

	static ros::Publisher& getAdder(){
		static ros::Publisher p = ros::NodeHandle().advertise<std_msgs::String>("/scriptable_analyzer/add_script", 100);
		return p;
	}
	static ros::Publisher& getRemover(){
		static ros::Publisher p = ros::NodeHandle().advertise<std_msgs::String>("/scriptable_analyzer/remove_script", 100);
		return p;
	}


	RosConstraints(string name, string script)
	: script_name(name), script(script)
	{
		std::stringstream st; st<<"#! name "<<name<<"\n#! type predicate\n"<<script;
		std_msgs::String msg; msg.data = preproc(st.str());
		getAdder().publish(msg);
	}
	~RosConstraints(){
		std_msgs::String msg; msg.data = script_name;
		getRemover().publish(msg);
	}


	std::string preproc(std::string txt)const;

private:
	std::string script_name;
	std::string script;
};

#define CONSTRAINTS(NAME, SCRIPT) \
		RosConstraints ros_constraints_##NAME(call_ctx.str()+"/"#NAME, #SCRIPT);






void ros_decision_making_init(int &argc, char **argv);

#endif /* ROSTASK_H_ */