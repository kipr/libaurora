#include <cstring>
#include <iostream>

#include <daylite/spinner.hpp>

#include "daylite-node.hpp"

using namespace aurora;
using namespace daylite;

daylite_node daylite_node::_instance;

daylite_node::daylite_node()
  : _frame_pub(nullptr)
{
}

daylite_node::~daylite_node()
{
  end();
}

bool daylite_node::start()
{
  if(_node)
  {
    std::cerr << "daylite_node is already initialized" << std::endl;
    return false;
  }

  if(!(_node = node::create_node("aurora")))
  {
    std::cerr << "node::create_node failed failed" << std::endl;
    return false;
  }

  if(!_node->start_gateway_service("127.0.0.1", 8374))
  {
    std::cerr << "_node->start_gateway_service failed" << std::endl;
    return false;
  }

  if(!(_frame_pub = _node->advertise("/aurora/frame")))
  {
    std::cerr << "_node->subscribe('/aurora/frame') failed" << std::endl;
    return false;
  }

  if(!(_key_events_sub = _node->subscribe("/aurora/key"
    , [](const bson_t* msg, void *usr_arg)
  {
    static_cast<daylite_node*>(usr_arg)->key_events_callback(msg);
  }
  , this)))
  {
    std::cerr << "_node->subscribe('/aurora/key') failed" << std::endl;
    return false;
  }

  if(!(_mouse_events_sub = _node->subscribe("/aurora/mouse"
    , [](const bson_t* msg, void *usr_arg)
  {
    static_cast<daylite_node*>(usr_arg)->mouse_events_callback(msg);
  }
  , this)))
  {
    std::cerr << "_node->subscribe('/aurora/mouse') failed" << std::endl;
    return false;
  }

  return true;
}

bool daylite_node::spin_once()
{
  return daylite::spinner::spin_once();
}

bool daylite_node::end()
{
  _key_events_sub.reset();
  _mouse_events_sub.reset();
  _frame_pub.reset();
  _node.reset();

  return true;
}

bool daylite_node::publish_frame(const char *type, const std::vector<uint8_t>& data)
{
  /* msg:
  *  {
  *    "type": <utf8>
  *    "data": <binary>
  *  }
  *
  */

  bson_t doc;
  bson_init(&doc);
  BSON_APPEND_UTF8(&doc, "type", type);
  BSON_APPEND_BINARY(&doc, "data", BSON_SUBTYPE_BINARY, &data[0], data.size());

  return _frame_pub->publish(&doc);
}

void daylite_node::key_events_callback(const bson_t* msg)
{
  /* msg:
  *  {
  *    "key_pressed": [ <key_code>, ... ]
  *  }
  *
  */

  std::unordered_set<KeyCode> pressed_keys;

  bson_iter_t it;
  bson_iter_t it_key_codes;
  if(bson_iter_init(&it, msg) && bson_iter_find(&it, "key_pressed")
    && BSON_ITER_HOLDS_ARRAY(&it) && bson_iter_recurse(&it, &it_key_codes))
  {
    while(bson_iter_next(&it_key_codes))
    {
      if(BSON_ITER_HOLDS_INT32(&it_key_codes))
      {
        int32_t value = bson_iter_int32(&it_key_codes);
        pressed_keys.emplace(static_cast<KeyCode>(value));
      }
      else
      {
        std::cerr << "Found unexpected object in 'key_pressed'" << std::endl;
      }
    }
  }
  else
  {
    std::cerr << "Message has no 'key_pressed' object" << std::endl;
  }

  if(_key_events_callback)
  {
    _key_events_callback(pressed_keys);
  }
}

bool get_int32_child(bson_iter_t *it, const char *dotkey, /* out */ int32_t& value)
{
  bson_iter_t it_child;
  if(bson_iter_find_descendant(it, dotkey, &it_child) && BSON_ITER_HOLDS_INT32(&it_child))
  {
    value = bson_iter_int32(&it_child);
    return true;
  }

  return false;
}

bool get_bool_child(bson_iter_t *it, const char *dotkey, /* out */ bool& value)
{
  bson_iter_t it_child;
  if(bson_iter_find_descendant(it, dotkey, &it_child) && BSON_ITER_HOLDS_BOOL(&it_child))
  {
    value = bson_iter_bool(&it_child);
    return true;
  }

  return false;
}

void daylite_node::mouse_events_callback(const bson_t* msg)
{
  /* msg:
   *  {
   *    "pos": {
   *      "x": <int32>
   *      "y": <int32>,
   *    },
   *    "button_down": {
   *      "left": <bool>,
   *      "middle": <bool>,
   *      "right": <bool>
   *    }
   *  }
   *
   */

  int32_t pos_x, pos_y;
  bool left_down, middle_down, right_down;
  bson_iter_t it;

  // pos.x:
  if(!(bson_iter_init(&it, msg) && get_int32_child(&it, "pos.x", pos_x)))
  {
    std::cerr << "Message has no 'pos.x' object" << std::endl;
  }

  // pos.y:
  if(!(bson_iter_init(&it, msg) && get_int32_child(&it, "pos.y", pos_y)))
  {
    std::cerr << "Message has no 'pos.y' object" << std::endl;
  }

  // button_down.left
  if(!(bson_iter_init(&it, msg) && get_bool_child(&it, "button_down.left", left_down)))
  {
    std::cerr << "Message has no 'buttons.left' object" << std::endl;
  }

  // button_down.middle
  if(!(bson_iter_init(&it, msg) && get_bool_child(&it, "button_down.middle", middle_down)))
  {
    std::cerr << "Message has no 'buttons.left' object" << std::endl;
  }

  // button_down.right
  if(!(bson_iter_init(&it, msg) && get_bool_child(&it, "button_down.right", right_down)))
  {
    std::cerr << "Message has no 'buttons.left' object" << std::endl;
  }

  if(_mouse_event_callback)
  {
    _mouse_event_callback(pos_x, pos_y, left_down, middle_down, right_down);
  }
}
