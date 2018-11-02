#include <ScriptObject.h>
#include <Actor/Player.h>

using namespace Core;

class CmnDefHousingSignboard :
  public EventScript
{
public:
  CmnDefHousingSignboard() :
    EventScript( 721031 )
  {
  }

  void Scene00000( Entity::Player& player )
  {
    player.playScene( getId(), 0, HIDE_HOTBAR, 0, 0 );
  }

  void onTalk( uint32_t eventId, Entity::Player& player, uint64_t actorId ) override
  {
    Scene00000( player );
  }
};