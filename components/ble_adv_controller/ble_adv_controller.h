#pragma once

#include "esphome/core/component.h"
#include "esphome/core/entity_base.h"
#include "esphome/core/helpers.h"
#include "esphome/core/preferences.h"
#ifdef USE_API
#include "esphome/components/api/custom_api_device.h"
#endif
#include "ble_adv_handler.h"
#include <vector>
#include <list>

namespace esphome {
namespace bleadvcontroller {


/**
  BleAdvController:
    One physical device controlled == One Controller.
    Referenced by Entities as their parent to perform commands.
    Chooses which encoder(s) to be used to issue a command
    Interacts with the BleAdvHandler for Queue processing
 */
class BleAdvController : public Component, public EntityBase
#ifdef USE_API
  , public api::CustomAPIDevice
#endif
{
public:
  void setup() override;
  void loop() override;
  virtual void dump_config() override;
  float get_setup_priority() const override { return 300.0f; }
  
  void set_min_tx_duration(int tx_duration, int min, int max, int step);
  uint32_t get_min_tx_duration() { return this->min_tx_duration_; }
  void set_max_tx_duration(uint32_t tx_duration) { this->max_tx_duration_ = tx_duration; }
  void set_seq_duration(uint32_t seq_duration) { this->seq_duration_ = seq_duration; }
  void set_forced_id(uint32_t forced_id) { this->params_.id_ = forced_id; }
  void set_forced_id(const std::string & str_id) { this->params_.id_ = fnv1_hash(str_id); }
  void set_index(uint8_t index) { this->params_.index_ = index; }
  void set_encoding_and_variant(const std::string & encoding, const std::string & variant);
  void set_reversed(bool reversed) { this->reversed_ = reversed; }
  bool is_reversed() const { return this->reversed_; }
  bool is_supported(const Command &cmd) { return this->cur_encoder_->is_supported(cmd); }
  void set_show_config(bool show_config) { this->show_config_ = show_config; }
  void set_handler(BleAdvHandler * handler) { this->handler_ = handler; }

#ifdef USE_API
  // Services
  void on_pair();
  void on_unpair();
  void on_cmd(float cmd, float arg0, float arg1, float arg2, float arg3);
  void on_raw_inject(std::string raw);
#endif

  bool enqueue(Command &cmd);

protected:

  uint32_t max_tx_duration_ = 3000;
  uint32_t seq_duration_ = 150;

  ControllerParam_t params_;

  bool reversed_;

  bool show_config_{false};
  BleAdvEncoder * cur_encoder_{nullptr};
  uint32_t min_tx_duration_{200};
  BleAdvHandler * handler_{nullptr};

  class QueueItem {
  public:
    QueueItem(CommandType cmd_type): cmd_type_(cmd_type) {}
    CommandType cmd_type_;
    std::vector< BleAdvParam > params_;
  
    // Only move operators to avoid data copy
    QueueItem(QueueItem&&) = default;
    QueueItem& operator=(QueueItem&&) = default;
  };
  std::list< QueueItem > commands_;

  // Being advertised data properties
  uint32_t adv_start_time_ = 0;
  uint16_t adv_id_ = 0;
};

/**
  BleAdvEntity: 
    Base class for implementation of Entities, referencing the parent BleAdvController
 */
class BleAdvEntity: public Component, public Parented < BleAdvController >
{
  public:
    virtual void dump_config() override = 0;

  protected:
    void dump_config_base(const char * tag);
    void command(CommandType cmd, const std::vector<uint8_t> &args);
    void command(CommandType cmd, uint8_t value1 = 0, uint8_t value2 = 0);
};

} //namespace bleadvcontroller
} //namespace esphome
