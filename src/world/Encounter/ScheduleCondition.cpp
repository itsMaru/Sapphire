#include "ScheduleCondition.h"

#include "EncounterTimeline.h"
#include "TimelineActor.h"
#include "TimelineActorState.h"

#include <Action/Action.h>

#include <Actor/Chara.h>
#include <Actor/BNpc.h>
#include <Actor/Player.h>

#include <Territory/Territory.h>
#include <Territory/InstanceContent.h>
#include <Territory/QuestBattle.h>

namespace Sapphire::Encounter
{
  bool ConditionHp::isConditionMet( ConditionState& state, TimelinePack& pack, TerritoryPtr pTeri, uint64_t time ) const
  {
    auto pBNpc = pTeri->getActiveBNpcByLayoutId( layoutId );
    if( !pBNpc )
      return false;

    // todo: check time elapsed

    switch( m_conditionType )
    {
      case ConditionType::HpPctLessThan:
        return pBNpc->getHpPercent() < hp.val;
      case ConditionType::HpPctBetween:
      {
        auto hpPct = pBNpc->getHpPercent();
        return hpPct > hp.min && hpPct < hp.max;
      }
    }
    return false;
  };

  bool ConditionDirectorVar::isConditionMet( ConditionState& state, TimelinePack& pack, TerritoryPtr pTeri, uint64_t time ) const
  {

    Event::DirectorPtr pDirector = pTeri->getAsInstanceContent();
    if( pDirector == nullptr )
      pDirector = pTeri->getAsQuestBattle();

    switch( m_conditionType )
    {
      case ConditionType::DirectorVarEquals:
        return pDirector->getDirectorVar( param.index ) == param.value;
      case ConditionType::DirectorVarGreaterThan:
        return pDirector->getDirectorVar( param.index ) > param.value;
      case ConditionType::DirectorFlagsEquals:
        return pDirector->getFlags() == param.flags;
      case ConditionType::DirectorFlagsGreaterThan:
        return pDirector->getFlags() > param.flags;
      case ConditionType::DirectorSeqEquals:
        return pDirector->getSequence() == param.seq;
      case ConditionType::DirectorSeqGreaterThan:
        return pDirector->getSequence() > param.seq;
    }
    return false;
  }

  bool ConditionCombatState::isConditionMet( ConditionState& state, TimelinePack& pack, TerritoryPtr pTeri, uint64_t time ) const
  {
    auto pBNpc = pTeri->getActiveBNpcByLayoutId( this->layoutId );

    if( !pBNpc )
      return false;

    // todo: these should really use callbacks when the state transitions or we could miss this tick
    switch( combatState )
    {
      case CombatStateType::Idle:
        return pBNpc->getState() == Entity::BNpcState::Idle;
      case CombatStateType::Combat:
        return pBNpc->getState() == Entity::BNpcState::Combat;
      case CombatStateType::Retreat:
        return pBNpc->getState() == Entity::BNpcState::Retreat;
      case CombatStateType::Roaming:
        return pBNpc->getState() == Entity::BNpcState::Roaming;
      case CombatStateType::JustDied:
        return pBNpc->getState() == Entity::BNpcState::JustDied;
      case CombatStateType::Dead:
        return pBNpc->getState() == Entity::BNpcState::Dead;
      default:
        break;
    }
    return false;
  }

  bool ConditionEncounterTimeElapsed::isConditionMet( ConditionState& state, TimelinePack& pack, TerritoryPtr pTeri, uint64_t time ) const
  {
    auto elapsed = time - pack.getStartTime();
    // todo: check encounter time
    return elapsed >= this->duration;
  }

  bool ConditionBNpcFlags::isConditionMet( ConditionState& state, TimelinePack& pack, TerritoryPtr pTeri, uint64_t time ) const
  {
    auto pBNpc = pTeri->getActiveBNpcByLayoutId( this->layoutId );
    return pBNpc && pBNpc->hasFlag( this->flags );
  }

  bool ConditionGetAction::isConditionMet( ConditionState& state, TimelinePack& pack, TerritoryPtr pTeri, uint64_t time ) const
  {
    auto pBNpc = pTeri->getActiveBNpcByLayoutId( this->layoutId );
    return pBNpc && pBNpc->getCurrentAction() && pBNpc->getCurrentAction()->getId() == this->actionId;
  }

  bool ConditionScheduleActive::isConditionMet( ConditionState& state, TimelinePack& pack, TerritoryPtr pTeri, uint64_t time ) const
  {
    return pack.isScheduleActive( this->actorName, this->scheduleName );
  }

  void ConditionHp::from_json( nlohmann::json& json, Schedule& phase, ConditionType condition,
                               const std::unordered_map< std::string, TimelineActor >& actors )
  {
    ScheduleCondition::from_json( json, phase, condition, actors );

    auto& paramData = json.at( "paramData" );
    auto actorRef = paramData.at( "sourceActor" ).get< std::string >();

    // resolve the actor whose hp we are checking
    if( auto it = actors.find( actorRef ); it != actors.end() )
      this->layoutId = it->second.m_layoutId;
    else
      throw std::runtime_error( fmt::format( std::string( "ConditionHp::from_json unable to find actor by name: %s" ), actorRef ) );

    switch( condition )
    {
      case ConditionType::HpPctLessThan:
        this->hp.val = paramData.at( "hp" ).get< uint32_t >();
        break;
      case ConditionType::HpPctBetween:
        this->hp.min = paramData.at( "hpMin" ).get< uint32_t >(),
        this->hp.max = paramData.at( "hpMax" ).get< uint32_t >();
        break;
      default:
        break;
    }
  }

  void ConditionDirectorVar::from_json( nlohmann::json& json, Schedule& phase, ConditionType condition,
                                        const std::unordered_map< std::string, TimelineActor >& actors )
  {
    ScheduleCondition::from_json( json, phase, condition, actors );

    auto& paramData = json.at( "paramData" );

    switch( condition )
    {
      case ConditionType::DirectorVarEquals:
      case ConditionType::DirectorVarGreaterThan:
      {
        param.index = paramData.at( "idx" ).get< uint32_t >();
        param.value = paramData.at( "val" ).get< uint32_t >();
      }
      break;
      case ConditionType::DirectorFlagsEquals:
      case ConditionType::DirectorFlagsGreaterThan:
      {
        param.flags = paramData.at( "flags" ).get< uint32_t >();
      }
      break;
      case ConditionType::DirectorSeqEquals:
      case ConditionType::DirectorSeqGreaterThan:
      {
        param.seq = paramData.at( "seq" ).get< uint32_t >();
      }
      break;
      default:
        break;
    }
  }

  void ConditionCombatState::from_json( nlohmann::json& json, Schedule& phase, ConditionType condition,
                                        const std::unordered_map< std::string, TimelineActor >& actors )
  {
    ScheduleCondition::from_json( json, phase, condition, actors );

    auto& paramData = json.at( "paramData" );
    auto actorRef = paramData.at( "sourceActor" ).get< std::string >();

    // resolve the actor whose name we are checking
    if( auto it = actors.find( actorRef ); it != actors.end() )
      this->layoutId = it->second.m_layoutId;
    else
      throw std::runtime_error( fmt::format( std::string( "ConditionCombatState::from_json unable to find actor by name: %s" ), actorRef ) );

    this->combatState = paramData.at( "combatState" ).get< CombatStateType >();
  }

  void ConditionEncounterTimeElapsed::from_json( nlohmann::json& json, Schedule& phase, ConditionType condition,
                                                 const std::unordered_map< std::string, TimelineActor >& actors )
  {
    ScheduleCondition::from_json( json, phase, condition, actors );

    auto& paramData = json.at( "paramData" );
    auto duration = paramData.at( "duration" ).get< uint64_t >();

    this->duration = duration;
  }

  void ConditionBNpcFlags::from_json( nlohmann::json& json, Schedule& phase, ConditionType condition,
                                      const std::unordered_map< std::string, TimelineActor >& actors )
  {
    ScheduleCondition::from_json( json, phase, condition, actors );
    auto& paramData = json.at( "paramData" );
    auto actorRef = paramData.at( "sourceActor" ).get< std::string >();

    // resolve the actor whose name we are checking
    if( auto it = actors.find( actorRef ); it != actors.end() )
      this->layoutId = it->second.m_layoutId;
    else
      throw std::runtime_error( fmt::format( std::string( "ConditionBNpcFlags::from_json unable to find actor by name: %s" ), actorRef ) );

    this->flags = json.at( "flags" ).get< uint32_t >();
    // todo: BNpcHasFlags
  }

  void ConditionGetAction::from_json( nlohmann::json& json, Schedule& phase, ConditionType condition,
                                      const std::unordered_map< std::string, TimelineActor >& actors )
  {
    ScheduleCondition::from_json( json, phase, condition, actors );

    auto& paramData = json.at( "paramData" );
    auto actorRef = paramData.at( "sourceActor" ).get< std::string >();
    auto actionId = paramData.at( "actionId" ).get< uint32_t >();

    // resolve the actor whose name we are checking
    if( auto it = actors.find( actorRef ); it != actors.end() )
      this->layoutId = it->second.m_layoutId;
    else
      throw std::runtime_error( fmt::format( std::string( "ConditionGetAction::from_json unable to find actor by name: %s" ), actorRef ) );

    this->actionId = actionId;
  }

  void ConditionScheduleActive::from_json( nlohmann::json& json, Schedule& phase, ConditionType condition,
                                        const std::unordered_map< std::string, TimelineActor >& actors )
  {
    ScheduleCondition::from_json( json, phase, condition, actors );

    auto& paramData = json.at( "paramData" );
    auto actorRef = paramData.at( "sourceActor" ).get< std::string >();
    auto scheduleName = paramData.at( "scheduleName" ).get< std::string >();

    this->actorName = actorRef;
    this->scheduleName = scheduleName;
  }

  // todo: i wrote this very sleep deprived, ensure it is actually sane

  void Schedule::execute( ConditionState& state, TimelineActor& self, TimelinePack& pack, TerritoryPtr pTeri, uint64_t time ) const
  {
    if( state.m_startTime == 0 )
    {
      state.m_startTime = time;
      self.spawnAllSubActors( pTeri );
    }

    if( state.m_scheduleInfo.m_startTime == 0 )
      state.m_scheduleInfo.m_startTime = time;

    if( state.m_scheduleInfo.m_lastTimepointTime == 0 )
      state.m_scheduleInfo.m_lastTimepointTime = time;

    for( auto i = state.m_scheduleInfo.m_lastTimepointIndex; i < m_timepoints.size();  )
    {
      auto elapsed = time - state.m_scheduleInfo.m_startTime;
      const auto& timepoint = m_timepoints[ i ];

      if( elapsed >= timepoint.m_offset )
      {
        state.m_scheduleInfo.m_lastTimepointTime = time;
        state.m_scheduleInfo.m_lastTimepointIndex = ++i;

        timepoint.execute( self, pack, pTeri, time );
        continue;
      }
      break;
    }
  }

  void Schedule::reset( ConditionState& state ) const
  {
    state.m_scheduleInfo.m_startTime = 0;
    state.m_scheduleInfo.m_lastTimepointIndex = 0;
    state.m_scheduleInfo.m_lastTimepointTime = 0;
  }

  bool Schedule::completed( const ConditionState& state ) const
  {
    return state.m_scheduleInfo.m_lastTimepointIndex == m_timepoints.size();
  }
}// namespace Sapphire::Encounter