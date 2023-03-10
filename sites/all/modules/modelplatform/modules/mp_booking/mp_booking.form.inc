<?php

/**
 * @file
 * Model Platform Booking - Forms.
 */

function mp_booking_job_model_invite_form_2($form, &$form_state, $model_uid, $customer_uid = 0) {
  global $user;

  $model_account = user_load($model_uid);

  // Works on models only.
  if (!isset($model_account->roles[MODELPLATFORM_ROLE_MODEL])) {
    drupal_goto(404);
  }

  $field_full_name = field_get_items('user', $model_account, 'field_full_name');
  if (isset($field_full_name[0]['value'])) {
    $full_name = $field_full_name[0]['value'];
    $title_params = array(
      '@model_name' => $full_name,
    );
  }
  else {
    $title_params = array(
      '@model_name' => $model_account->name,
    );
  }
  $title = t('Book of @model_name', $title_params);
  drupal_set_title($title);

    $customer_uid = $user->uid; 
  // Get model unit.
  $unit = _mp_booking_get_unit('user', $model_account);
  $form_state['#unit'] = $unit;

  $customer_jobs = _mp_booking_get_opened_customer_jobs($customer_uid);

  $defult_job_nid = NULL;
  $count_jobs = count($customer_jobs);
  if (true) {
    if ($count_jobs == 1) {
      $nids = array_keys($customer_jobs);
      $defult_job_nid = $nids[0];
    }

    $auctioned_job = _mp_booking_get_auctioned_job($model_uid, $customer_uid);

    if ($auctioned_job) {
        $start_date = $end_date = NULL;

        // Job dates.
        if (isset($auctioned_job->field_job_start_date[LANGUAGE_NONE][0]['value'])) {
          $start_date = $auctioned_job->field_job_start_date[LANGUAGE_NONE][0]['value'];
        }
        if (isset($auctioned_job->field_job_start_date[LANGUAGE_NONE][0]['value2'])) {
          $end_date = $auctioned_job->field_job_start_date[LANGUAGE_NONE][0]['value2'];
        }
        $model_events = _mp_booking_get_model_events($model_uid, $start_date, $end_date);
        $keys = array_keys($model_events);
        $idx = array_search($auctioned_job->nid, $keys);
        $event_id = $model_events[$auctioned_job->nid];
        $event = bat_event_load($event_id);

        $status = NULL;
        $decision = '';

        $auction_data = mp_booking_get_event_last_auction($event_id);

        $auction = mp_auction_load($auction_data->auction_id);
        $field_status = field_get_items('mp_auction', $auction, 'field_auction_status');
        $status_items = array();
        if (isset($field_status[0]['value'])) {
          $field = field_info_field('field_auction_status');
          $values = list_allowed_values($field);

          // @TODO Should be changed. Hot fix.
          unset($values['request sent']);
          unset($values['confirmed']);

          $value = $field_status[0]['value'];
          if ($value !== 'read') {
            $value = 'unread';
          }
          foreach($values as $key => $status) {
            $class = '';
            if ($key == $value) {
              $class = 'job-customer-decision';
            }
            $status_items[] = '<span class="' . $class . '">' . t($status) . '</span>';
          }
        }
        $field_decision = field_get_items('mp_auction', $auction, 'field_auction_decision');
        $decision_items = array();
        $decision_value = FALSE;
        $decision_field = field_info_field('field_auction_decision');
        $decision_values = list_allowed_values($decision_field);
        if (isset($field_decision[0]['value'])) {
          $decision_value = $field_decision[0]['value'];
        }
        if (!$decision_value) {
          $decision_value = 'Request sent';
        }
        foreach($decision_values as $key => $status) {
          $class = $prefix = $suffix = '';
          if ($key == $decision_value) {
            $class = 'job-customer-decision';
            $prefix = '<b>';
            $suffix = '</b>';
          }
          $decision_items[] = '<span class="' . $class . '">' . $prefix . t($status) . $suffix . '</span>';
        }

        $form['job_title'] = array(
          '#type' => 'markup',
          '#markup' => '<h5>' . $auctioned_job->title . '</h5>',
        );
        $form['auction_form'] = array(
          '#type' => 'markup',
          '#markup' => views_embed_view('job_propose_auctions', 'block', array($event_id)),
        );
        $form['decision'] = array(
          '#type' => 'markup',
          '#prefix' => '<div class="auction-decision-new">',
          '#markup' => implode(' ', $decision_items),
          '#suffix' => '</div>',
        );
        $form['status'] = array(
          '#type' => 'markup',
          '#markup' => '<div class="auction-status">' . implode(' ', $status_items) . '</div>',
        );
        $form['auction_id'] = array(
          '#type' => 'value',
          '#value' => $auction_data->auction_id,
        );
        $form['event_id'] = array(
          '#type' => 'value',
          '#value' => $event_id,
        );

        $form['jobs'] = array(
          '#type' => 'value',
          '#value' => $auctioned_job->nid,
        );

        foreach ($keys as $key => $event_key) {
          $class = $prefix = $suffix = '';
          if ($key == $idx) {
            $class = 'job-customer-decision';
            if ($key == 0) {
              $prefix = '<b>';
              $suffix = '</b>';
            }
          }
          $items[]=null;
          $items[] = '<span class="' . $class . '">' . $prefix . t('Shortlist @option_num', array('@option_num' => $key + 1)) . $suffix . '</span>';
        }

        $form['job_number'] = array(
          '#type' => 'markup',
          '#prefix' => '<div class="job-number">',
          '#markup' => implode(' ', $items),
          '#suffix' => '</div>',
        );
        $enabled = (
              !isset($auction->field_auction_decision[LANGUAGE_NONE][0]['value'])
          &&  $auction->field_decision_user[LANGUAGE_NONE][0]['target_id'] == $user->uid
          )
          || (
              isset($auction->field_auction_decision[LANGUAGE_NONE][0]['value'])
          &&  $auction->field_auction_decision[LANGUAGE_NONE][0]['value'] == 'Accept'
          &&  $auction->field_decision_user[LANGUAGE_NONE][0]['target_id'] != $user->uid
        );

        $form['actions'] = array(
          '#type' => 'container',
          '#tree' => TRUE,
        );
        $query = db_select('model_job', 'mj');
        $query->fields('mj', array('job_status'));
        $query->condition('uid',$model_uid,'=');
        $query->condition('job_id',$auctioned_job->nid,'=');
        $result = $query->execute()->fetchAssoc();

        if ($enabled && $auctioned_job->field_job_status[LANGUAGE_NONE][0]['tid'] !=97) {

          if($result['job_status'] =='accepted'){

            $order_id = $event->field_order['und'][0]['target_id'];
            $order =commerce_order_load($order_id);
            if($order->status =='job_invoice_status'){

              $_SESSION['payment_goto_booking_link'] = current_path();
              $form['actions']['payment'] = array(
                    '#type' => 'link',
                    '#title' => t('Make Payment'),
                    '#href' => 'user/'.$user->uid.'/invoices/'.$order_id,
                    '#id' => 'payment-link',
                    '#attributes' => array('class' => array('btn-medium btn btn-mod btn-round')),
                    '#options' => array(
                      'html' => true,
                    )
                  );
            } else{
              $form['actions']['book'] = array(
                '#type' => 'submit',
                '#value' => t('Book'),
                '#name' => 'job_book',
                '#disabled' => !$enabled,
                '#submit' => array(
                  'mp_booking_job_model_invite_form_confirm_submit',
                ),
              );
            }
          } else {
            $form['actions']['confirm'] = array(
              '#type' => 'submit',
              '#value' => t('Shortlist '),
              '#name' => 'job_confirm',
              '#disabled' => !$enabled,
              '#submit' => array(
                'mp_booking_job_model_optioned_form_submit',
              ),
            );
          }
          $form['actions']['cancel'] = array(
            '#type' => 'submit',
            '#value' => t('Cancel'),
            '#name' => 'job_cancel',
            '#disabled' => !$enabled,
            '#submit' => array(
              'mp_booking_job_model_invite_form_cancel_submit',
            ),
          );
        }
      }
  }
  else {
    $form['info'] = array(
      '#type' => 'markup',
      '#markup' => t('You have no active Jobs'),
    );
  }

  return $form;
}

/**
 * Model job invitiation form.
 */
function mp_booking_job_model_invite_form($form, &$form_state, $model_uid, $customer_uid = 0) {
  global $user;

  $model_account = user_load($model_uid);

  // Works on models only.
  if (!isset($model_account->roles[MODELPLATFORM_ROLE_MODEL])) {
    drupal_goto(404);
  }

  $field_full_name = field_get_items('user', $model_account, 'field_full_name');
  if (isset($field_full_name[0]['value'])) {
    $full_name = $field_full_name[0]['value'];
    $title_params = array(
      '@model_name' => $full_name,
    );
  }
  else {
    $title_params = array(
      '@model_name' => $model_account->name,
    );
  }
  $title = t('Book of @model_name', $title_params);
  drupal_set_title($title);

  if (!is_numeric($customer_uid) || !$customer_uid) {
    $customer_uid = $user->uid;
  }

  // Get model unit.
  $unit = _mp_booking_get_unit('user', $model_account);
  $form_state['#unit'] = $unit;

  $customer_jobs = _mp_booking_get_opened_customer_jobs($customer_uid);
  $auctioned_jobs = _mp_booking_get_auction_event_data_2($model_uid, $customer_uid);
  foreach ($auctioned_jobs as $job) {
    unset($customer_jobs[$job->job_nid]);
  }
  $nid = $_GET['node'];
  $defult_job_nid = $nid;
  $count_jobs = count($customer_jobs);
  if ($count_jobs) {
    if ($count_jobs == 1) {
      $nids = array_keys($customer_jobs);
      $defult_job_nid = $nids[0];
    }
      $form['model_uid'] = array(
        '#type' => 'value',
        '#value' => $model_uid,
      );

      if ($count_jobs == 1) {
        $form['jobs'] = array(
          '#type' => 'value',
          '#value' => $defult_job_nid,
        );
      }
      else {
        $form['jobs'] = array(
          '#type' => 'select',
          '#options' => $customer_jobs,
          '#default_value' => $nid,
        );
      }

      $form['job_invite'] = array(
        '#type' => 'submit',
        '#value' => t('Invite Please'),
        '#name' => "job_invite",
        '#submit' => array(
          'mp_booking_job_model_invite_form_submit',
        ),
        '#validate' => array(
          'mp_booking_job_model_invite_form_validate',
        ),
      );
      $form['#validate'] = 'mp_booking_job_model_invite_form_validate';
  }
  else {
    $form['info'] = array(
      '#type' => 'markup',
      '#markup' => t('You have no active Jobs'),
    );
  }

  return $form;
}

/**
 * Form validate.
 *
 * @see mp_booking_job_model_invite_form()
 */
function mp_booking_job_model_invite_form_validate($form, &$form_state) {
  $values = $form_state['values'];

  $job_nid = $values['jobs'];
  $job = node_load($job_nid);

  $start_date = $end_date = NULL;

  // Job dates check.
  if (!isset($job->field_job_start_date[LANGUAGE_NONE][0]['value'])) {
    form_set_error('jobs', t('Job start date is empty'));
  }
  else {
    $start_date = $job->field_job_start_date[LANGUAGE_NONE][0]['value'];
  }
  if (!isset($job->field_job_start_date[LANGUAGE_NONE][0]['value2'])) {
    form_set_error('jobs', t('Job end date is empty'));
  }
  else {
    $end_date = $job->field_job_start_date[LANGUAGE_NONE][0]['value2'];
  }

  // Model availability check.
  if ($start_date && $end_date) {
    // Get unit events.
    $events = _mp_booking_get_unit_events($form_state['#unit'], $start_date, $end_date, 1, FALSE);

    // Check events.
    if (count($events)) {
      form_set_error('jobs', t('Model is already busy on this dates.'));
    }
  }
}

/**
 * Form submit.
 *
 * @see mp_booking_job_model_invite_form()
 */
function mp_booking_job_model_invite_form_submit($form, &$form_state) {
  global $user;

  $values = $form_state['values'];

  $job_nid = $values['jobs'];
  $job = node_load($job_nid);

  $start_date = $job->field_job_start_date[LANGUAGE_NONE][0]['value'];
  $end_date = $job->field_job_start_date[LANGUAGE_NONE][0]['value2'];

  // Get model unit.
  $model_account = user_load($values['model_uid']);
  $unit = _mp_booking_get_unit('user', $model_account);

  // Create event.
  $event = _mp_booking_event_create($unit->unit_id, MP_BOOKING_STATE_AUCTION, $user->uid, $job_nid, $start_date, $end_date);
  bat_event_save($event);

  // Create auction.
  module_load_include('inc', 'mp_auction', 'mp_auction_mp_auction.admin');
  $auction = mp_auction_create(array('type' => 'auction'));
  $auction->name = 'Auction - ' . $event->event_id . '[0]';
  $auction->uid = $user->uid;
  $auction->field_auction_event[LANGUAGE_NONE][0]['target_id'] = $event->event_id;
  $auction->field_auction_status[LANGUAGE_NONE][0]['value'] = MP_BOOKING_AUCTION_REQUEST_SENT;
  $auction->field_auction_cost_propose = $event->field_job_cost;
  $auction->field_decision_user[LANGUAGE_NONE][0]['target_id'] = $values['model_uid'];
  mp_auction_save($auction);

  db_insert('model_job')
    ->fields(array(
      'uid' => $values['model_uid'],
      'job_id' => $job_nid,
      'job_status' => 'invited'
    ))
    ->execute();

  module_invoke_all('mpb_option_model', $event);
  drupal_set_message(t('Model has been invited'));
}

function mp_booking_job_model_optioned_form_submit($form, &$form_state) {
  global $user;

  $values = $form_state['values'];
  $mp_auction_id = $form_state['values']['auction_id'];
  $auction = mp_auction_load($mp_auction_id);

  $event = bat_event_load($form_state['values']['event_id']);
  $event->field_job_cost = $auction->field_auction_cost_propose;
  $event->event_state_reference[LANGUAGE_NONE][0]['state_id'] = 3;
  bat_event_save($event);

  $job_nid = $values['jobs'];
  $job = node_load($job_nid);

  $job->field_job_status[LANGUAGE_NONE][0]['tid'] = 96;
  node_save($job);

  db_update('model_job')
                ->fields(array('job_status' => 'accepted' ))
                ->condition('job_id', $job_nid, '=')
                ->condition('uid', arg(2), '=')
                ->execute();
  //module_invoke_all('mpb_option_model', $event);
  drupal_set_message(t('Model was Shortlisted'));
}
/**
 * Form submit.
 *
 * @see mp_booking_job_model_invite_form()
 */
function mp_booking_job_model_invite_form_confirm_submit($form, $form_state) {

  $event = bat_event_load($form_state['values']['event_id']);
  $order_id = $event->field_order['und'][0]['target_id'];
  $order =commerce_order_load($order_id);
  if($order->status == 'completed'){

    global $user;
    $mp_auction_id = $form_state['values']['auction_id'];
    $event_id = $form_state['values']['event_id'];
    $auction = mp_auction_load($mp_auction_id);
    $auction->field_auction_decision[LANGUAGE_NONE][0]['value'] = 'Accept';
    //$auction->field_decision_user[LANGUAGE_NONE][0]['target_id'] = $user->uid;
    
    mp_auction_save($auction);
    //module_invoke_all('mpb_option_model_accept', $auction);
    $email_params = _mp_emails_get_event_params($event_id);
    _mp_emails_model_job_confirmed($email_params);

    $event = bat_event_load($form_state['values']['event_id']);

    $event->field_job_cost = $auction->field_auction_cost_propose;
    $event->event_state_reference[LANGUAGE_NONE][0]['state_id'] = 3;

    bat_event_save($event);

    // Set booked status for the job.
    $values = $form_state['values'];
    $job_nid = $values['jobs'];

    $job = node_load($job_nid);
    $job_wrapper = entity_metadata_wrapper('node', $job);
    $job_wrapper->field_job_status = 97;
    $job_wrapper->save();

    _mp_booking_cancel_other_events($job_nid, $event_id);
  }else{
    drupal_set_message(t('You have not completed Payment yet.'), 'error');
  }
}

/**
 * Form submit.
 *
 * @see mp_booking_job_model_invite_form()
 */
function mp_booking_job_model_invite_form_cancel_submit($form, $form_state) {
  $mp_auction_id = $form_state['values']['auction_id'];
  $auction = mp_auction_load($mp_auction_id);
  $auction->field_auction_decision[LANGUAGE_NONE][0]['value'] = 'Decline';
  mp_auction_save($auction);
  module_invoke_all('mpb_option_model_decline', $auction);
}

/**
 * Model job auction.
 */
function mp_booking_job_auction_form($form, &$form_state, $event = NULL) {
  global $user;

  $create_new_auction = FALSE;
  $negotiations_count = 0;

  if (isset($form_state['triggering_element'])) {
    if ($form_state['triggering_element']['#name'] === 'auction_decision_new_propose') {
      $create_new_auction = TRUE;
    }
  }

  if (isset($event->event_id) && $event->event_id) {
    // Get owner and decision uids.
    $model_uid = 0;
    if (isset($event->event_bat_unit_reference[LANGUAGE_NONE][0]['target_id'])) {
      $unit = bat_unit_load($event->event_bat_unit_reference[LANGUAGE_NONE][0]['target_id']);
      $model_uid = $unit->uid;
    }
    $customer_uid = 0;
    if (isset($event->field_job[LANGUAGE_NONE][0]['target_id'])) {
      $job = node_load($event->field_job[LANGUAGE_NONE][0]['target_id']);
      $customer_uid = $job->uid;
    }
    if ($model_uid == $user->uid) {
      $decision_uid = $customer_uid;
      $owner_uid = $model_uid;
    }
    else {
      $decision_uid = $model_uid;
      $owner_uid = $customer_uid;
      $create_new_auction = FALSE;
    }

    // Get events auctions.
    $auctions = mp_booking_get_event_auctions($event);
    foreach ($auctions as $auction_item) {
      if ($auction_item->uid === $user->uid) {
        $negotiations_count++;

      }
    }
    $auction = end($auctions);

    if ((!isset($auction->field_auction_status[LANGUAGE_NONE][0]['value']) || $auction->field_auction_status[LANGUAGE_NONE][0]['value'] != 'read')) {
      $auction->field_auction_status[LANGUAGE_NONE][0]['value'] = 'read';
      mp_auction_save($auction);
    }

$enabled = (
              !isset($auction->field_auction_decision[LANGUAGE_NONE][0]['value'])
          &&  $auction->field_decision_user[LANGUAGE_NONE][0]['target_id'] == $user->uid
          )
          || (
              isset($auction->field_auction_decision[LANGUAGE_NONE][0]['value'])
          &&  $auction->field_auction_decision[LANGUAGE_NONE][0]['value'] != 'Accept'
          &&  $auction->field_decision_user[LANGUAGE_NONE][0]['target_id'] != $user->uid
        );

    $decision_form_show = FALSE;
    if ($enabled){
        $decision_form_show = TRUE;
      }
    
    else {
      if (isset($auction->field_auction_decision[LANGUAGE_NONE][0]['value'])
        && $auction->field_auction_decision[LANGUAGE_NONE][0]['value'] == 'Decline'
      ) {
        $decision_form_show = FALSE;
      }
    }

    if ($decision_form_show) {
      if ($create_new_auction) {
        $_auction = mp_auction_create(array('type' => 'auction'));
        $_auction->name = 'Auction - ' . $event->event_id . '[' . count($auctions) . ']';
        $_auction->uid = $owner_uid;
        $_auction->field_auction_event[LANGUAGE_NONE][0]['target_id'] = $event->event_id;
        $_auction->field_auction_cost_propose[LANGUAGE_NONE][0]['amount'] = $form_state['values']['field_auction_cost_propose'];
        $_auction->field_decision_user[LANGUAGE_NONE][0]['target_id'] = $decision_uid;
        $form = mp_auction_edit_form($form, $form_state, $_auction);
      }
      $form['#prefix'] = '<div id="job_auction_form">';
      $form['#suffix'] = '</div>';

      $form['#event'] = $event;

      $form['_mp_auction_id'] = array(
        '#type' => 'value',
        '#value' => $auction->mp_auction_id,
      );
      $form['decision'] = array(
        '#title' => t('Decision'),
        '#type' => 'container',
        '#tree' => TRUE,
        '#prefix' => '<div id="decision-wrapper" class="decision-propose">',
        '#suffix' => '</div>',
      );
      $form['decision']['accept'] = array(
        '#type' => 'submit',
        '#value' => t('Agree to the daily rate'),
        '#name' => 'auction_decision_accept',
        '#submit' => array(
          'mp_booking_job_auction_form_accept',
        ),
      );
      if ($owner_uid != $customer_uid
        && $negotiations_count < 3
      ) {
        $form['decision']['new_propose'] = array(
          '#type' => 'button',
          '#value' => t('Reject the daily Rate'),
          '#name' => 'auction_decision_new_propose',
          '#disabled' => $create_new_auction,
          '#ajax' => array(
            'callback' => 'mp_booking_job_auction_form_ajax',
            'wrapper' => 'job_auction_form',
            'method' => 'replace',
            'effect' => 'fade',
          ),
        );
      }
      else {
        $form['decision']['decline'] = array(
            '#markup' => '<div class="job-cancel-final">'.l(t('Cancel job'), 'job/cancel/'.$job->nid). '</div>',
        );
      }
      $form['decision']['description'] = array(
        '#type' => 'markup',
        '#markup' => '<span class="description">' .
          t('You can negotiate the rate up to @max_times times only. (@times left)',
            array(
              '@max_times' => MP_BOOKING_MAX_NEGOTIATIONS,
              '@times' => MP_BOOKING_MAX_NEGOTIATIONS - $negotiations_count
            )
          )
          . '</span>',
      );


      // Hide filled fields.
      $form['field_auction_decision']['#access'] = FALSE;
      $form['field_auction_event']['#access'] = FALSE;
      $form['name']['#access'] = FALSE;
      $form['actions']['delete']['#access'] = FALSE;
      $form['field_auction_status']['#access'] = FALSE;
      $form['field_decision_user']['#access'] = FALSE;

      if ($create_new_auction) {
        $form['actions']['submit']['#value'] = t('Send new propose');
        $form['actions']['submit']['#submit'][] = 'mp_booking_job_auction_form_submit';
      }
    }
    else {
      if (!isset($auction->field_auction_decision[LANGUAGE_NONE][0]['value'])
          || $auction->field_auction_decision[LANGUAGE_NONE][0]['value'] != 'Accept') {
        $customer = user_load($customer_uid);
        $customer_wrapper = entity_metadata_wrapper('user', $customer);
        $customer_full_name = $customer_wrapper->field_full_name->value();
        $form['decision']['description'] = array(
          '#type' => 'markup',
          '#markup' => '<span class="description">' .
            t('You should wait @customer_full_name decision.',
              array(
                '@customer_full_name' => $customer_full_name,
              )
            )
            . '</span>'
        );
      }
    }
  }

  return $form;
}

function mp_booking_job_auction_form_ajax($form, $form_state) {
  return $form;
}

/**
 * Submit action.
 *
 * @see mp_booking_job_auction_form()
 */
function mp_booking_job_auction_form_accept($form, &$form_state) {
  global $user;
  $auction_id = $form_state['values']['_mp_auction_id'];
  $auction = mp_auction_load($auction_id);
  $auction->field_auction_decision[LANGUAGE_NONE][0]['value'] = 'Accept';
  mp_auction_save($auction);

  module_invoke_all('mpb_option_model_accept', $auction);
}

/**
 * Submit action.
 *
 * @see mp_booking_job_auction_form()
 */
function mp_booking_job_auction_form_decline($form, &$form_state) {
  $auction_id = $form_state['values']['_mp_auction_id'];
  $auction = mp_auction_load($auction_id);
  $auction->field_auction_decision[LANGUAGE_NONE][0]['value'] = 'Decline';
  mp_auction_save($auction);

  module_invoke_all('mpb_option_model_decline', $auction);
}

/**
 * Submit action.
 *
 * @see mp_booking_job_auction_form()
 */
function mp_booking_job_auction_form_submit($form, &$form_state) {
  mp_booking_job_auction_form_decline($form, $form_state);

  $form_state['redirect'] = NULL;
}

/**
 * Booking form.
 */
function mp_booking_calendar_form($form, &$form_state, $account = NULL) {
  $event = NULL;
  if ($account) {
    // Get BAT Unit object.
    $unit = _mp_booking_get_unit('user', $account);

    // Create BAT event object.
    module_load_include('inc', 'bat_event', 'bat_event.admin');
    $event = _mp_booking_event_create($unit->unit_id, MP_BOOKING_STATE_BUSY, $account->uid, -1);
  }
  else {
    $args = arg();
    if (count($args) === 4 && $args[0] === 'busy' && $args[1] === 'event' && $args[3] === 'edit') {
      $event = bat_event_load(intval($args[2]));
    }
  }

  if ($event) {
    // Create form.
    $form = bat_event_edit_form($form, $form_state, $event);

    // Hide other fields.
    // @TODO Create module configuration for this fields.
    $form['user']['#access'] = FALSE;
    $form['event_history']['#access'] = FALSE;
    $form['event_bat_unit_reference']['#access'] = FALSE;
    $form['event_state_reference']['#access'] = FALSE;
    $form['field_job']['#access'] = FALSE;
    $form['field_job_cost']['#access'] = FALSE;
    $form['actions']['cancel']['#access'] = FALSE;

    $form['field_order']['#access'] = FALSE;
    $form['field_additional_receipts']['#access'] = FALSE;

    // Change date format.
    $date_format = 'Y-m-d h:i A';
    $date_year_range = '0:+1';

    $form['bat_date_range']['bat_start_date']['#date_format'] = $date_format;
    $form['bat_date_range']['bat_start_date']['#date_year_range'] = $date_year_range;

    $form['bat_date_range']['bat_end_date']['#date_format'] = $date_format;
    $form['bat_date_range']['bat_end_date']['#date_year_range'] = $date_year_range;

    // I had to disable the validation of the form, because BAT generates
    // new values for the start and end date, even if they have not been entered
    // by the user, which causes the error when form submit.
    $form['#validate'] = array(
      '_rcb_bat_event_form_validate',
    );
    $form['bat_date_range']['bat_start_date']['#element_validate'] = array('_rcb_bat_date_validate');
    $form['bat_date_range']['bat_end_date']['#element_validate'] = array('_rcb_bat_date_validate');

    $form['actions']['submit']['#submit'] = array('mp_booking_calendar_form_submit');
    $form['actions']['submit']['#value'] = t('Mark as unavailable');
  }

  return $form;
}

/**
 * Element validate callback for the bat_date_range fields.
 *
 * Ensure a start and end date was entered and provide default value for time.
 */
function _rcb_bat_date_validate($element, &$form_state, $form) {
  if (empty($element['#value']['date'])) {
    form_error($element, t('Please choose a @title.', array('@title' => $element['#title'])));
  }
  elseif (empty($element['#value']['time'])) {
    $form_state['values'][$element['#name']]['time'] = '10:00 AM';
  }
}

/**
 * BAT event additional form validation.
 *
 * @see mp_booking_calendar_form()
 */
function _rcb_bat_event_form_validate($form, &$form_state) {
  $error = FALSE;

  // Start date.
  if (!isset($form_state['input']['bat_start_date']['date'])
    || !$form_state['input']['bat_start_date']['date']) {
    $error = TRUE;
    form_set_error('bat_date_range][bat_start_date');
  }

  // End date.
  if (!isset($form_state['input']['bat_end_date']['date'])
    || !$form_state['input']['bat_end_date']['date']) {
    $error = TRUE;
    form_set_error('bat_date_range][bat_end_date');
  }

  // If default bat event validation will execute with date error
  // it will create fatal error, so we need to prevent it.
  if (!$error) {
    bat_form_start_end_dates_validate($form, $form_state);
    bat_event_edit_form_validate($form, $form_state);
  }
}

/**
 * Form submit.
 *
 * @see mp_booking_calendar_form()
 */
function mp_booking_calendar_form_submit($form, &$form_state) {
  // Call exists BAT submit function.
  // 
  bat_event_edit_form_submit($form, $form_state);
  $form_state['redirect'] = NULL;
  
    db_insert('field_data_event_state_reference') // Table name no longer needs {}
      ->fields(array(
        'entity_type' => 'bat_event',
        'bundle' => 'model_availability',
        'deleted' => 0,
        'entity_id' =>$form_state['bat_event']->event_id ,
        'revision_id' =>$form_state['bat_event']->event_id,
        'language' => 'en',
        'delta' => 0,
        'event_state_reference_state_id' => 2,
      ))
      ->execute();

      db_insert('field_revision_event_state_reference') // Table name no longer needs {}
      ->fields(array(
        'entity_type' => 'bat_event',
        'bundle' => 'model_availability',
        'deleted' => 0,
        'entity_id' =>$form_state['bat_event']->event_id ,
        'revision_id' =>$form_state['bat_event']->event_id,
        'language' => 'en',
        'delta' => 0,
        'event_state_reference_state_id' => 2,
      ))
      ->execute();
}

/**
 * Model job reciepts.
 */
function mp_booking_job_model_reciepts_form($form, &$form_state, $event) {
  if ($event) {
    $event_wrapper = entity_metadata_wrapper('bat_event', $event);
    $state_id = $event_wrapper->event_state_reference->value()['state_id'];
    $end_date = $event->end_date_object->getTimestamp();

    if (($state_id == MP_BOOKING_STATE_JOB_CONTRACT)
      && (REQUEST_TIME > $end_date)
      && ((REQUEST_TIME - $end_date) < 604800)
    ) {
      // Create form.
      $form = bat_event_edit_form($form, $form_state, $event);

      $form['user']['#access'] = FALSE;
      $form['event_history']['#access'] = FALSE;
      $form['event_bat_unit_reference']['#access'] = FALSE;
      $form['event_state_reference']['#access'] = FALSE;
      $form['field_job']['#access'] = FALSE;
      $form['field_order']['#access'] = FALSE;
      $form['field_job_cost']['#access'] = FALSE;
      $form['actions']['cancel']['#access'] = FALSE;
      $form['bat_date_range']['#disabled'] = TRUE;

      $fc_items = $event_wrapper->field_additional_receipts->value();
      $field_collection = &$form['field_additional_receipts'][LANGUAGE_NONE];
      foreach (element_children($field_collection) as $key) {
        if (!is_numeric($key)) {
          continue;
        }
        $disabled = FALSE;
        if (isset($fc_items[$key])) {
          $fc_item_wrapper = entity_metadata_wrapper('field_collection_item', $fc_items[$key]);
          $admin = $fc_item_wrapper->field_reciept_admin->value();
          $disabled = ($admin || $disabled);
          $field_collection[$key]['field_reciept_type']['#disabled'] = $disabled;
          $field_collection[$key]['field_document']['#disabled'] = $disabled;
          $field_collection[$key]['field_reciept_comment']['#disabled'] = $disabled;
          $field_collection[$key]['field_reciept_cost']['#disabled'] = $disabled;
        }

        $field_collection[$key]['field_reciept_approved_by_admin']['#access'] = FALSE;
        $field_collection[$key]['field_reciept_admin']['#access'] = FALSE;
        $field_collection[$key]['remove_button']['#value']= t('Remove row');
        if ($disabled) {
          $field_collection[$key]['remove_button']['#disabled'] = FALSE;
        }
      }

      $form['#validate'] = array(
        '_rcb_bat_event_form_validate',
      );
      $form['bat_date_range']['bat_start_date']['#element_validate'] = array('_rcb_bat_date_validate');
      $form['bat_date_range']['bat_end_date']['#element_validate'] = array('_rcb_bat_date_validate');

      $form['actions']['submit']['#submit'] = array('mp_booking_job_model_reciepts_form_submit');
      $form['actions']['submit']['#value'] = t('Save reciepts');
    }
  }

  return $form;
}

/**
 * Job model reciepts submit.
 *
 * @see mp_booking_job_model_reciepts_form()
 */
function mp_booking_job_model_reciepts_form_submit($form, &$form_state) {
  // Call exists BAT submit function.
  bat_event_edit_form_submit($form, $form_state);

  $form_state['redirect'] = NULL;
}

/**
 *
 */
function mp_booking_job_cancel_form($form, &$form_state, $job_nid) {
  $form['#job_nid'] = $job_nid;
  $form_state['#direct_path'] = '/dashboard';

  $cancel_path ='/dashboard';
  if (!empty($_SERVER['HTTP_REFERER']) OR isset($_SESSION['referrer'])) {
    
    if(!empty($_SERVER['HTTP_REFERER'])){
      $_SESSION['referrer'] = $_SERVER['HTTP_REFERER'];
    }

    $cancel_path = $_SESSION['referrer'];
  }

  return confirm_form(
    $form,
    t('Are you sure you want to reject the job contract?'),
    $cancel_path,
    t('This action cannot be undone.'),
    t('Accept'),
    t('Decline')
  );
}

function mp_booking_job_cancel_form_submit($form, &$form_state) {
  global $user;

  $job_nid = $form['#job_nid'];
  $job = node_load($job_nid);
  $job_wrapper = entity_metadata_wrapper('node', $job);
  $job_wrapper->field_job_status->set(99);
  $job_wrapper->save();

  $job_events = _mp_booking_get_job_events($job_nid);
  foreach ($job_events as $job_event_id) {
    $job_event = bat_event_load($job_event_id);
    $job_event->uid = $user->uid;
    $job_event->event_state_reference[LANGUAGE_NONE][0]['state_id'] = 6;

    bat_event_delete($job_event);
  }

  $form_state['redirect'] = $form_state['#direct_path'];
}

/**
 * Delete event form.
 */
function mp_booking_busy_event_delete_form($form, &$form_state, $event_id) {
  global $user;

  $form['caution'] = array(
    '#prefix' => '<div class="caution">',
    '#markup' => t('This action can not be undone.'),
    '#suffix' => '</div>',
  );

  $form['event_id'] = array(
    '#type' => 'value',
    '#value' => $event_id
  );
  $form['actions'] = array(
    '#type' => 'container',
    '#tree' => FALSE,
  );

  $form['actions']['delete'] = array(
    '#type' => 'submit',
    '#value' => t('Delete'),
    '#submit' => array(
      'mp_booking_busy_event_delete_form_submit',
    ),
  );

//  $form['actions']['cancel'] = array(
//    '#type' => 'markup',
//    '#markup' => l(t('Cancel'), '/user/' . $user->uid),
//  );

  $form['actions']['cancel'] = array(
        '#type' => 'link',
        '#title' => t('Cancel'),
        '#href' => '/user/' . $user->uid,
  );
  $form['actions']['cancel']['#attributes']['class'][] = 'event-delete-cancel btn-medium btn btn-mod btn-round';

  return $form;
}

/**
 * Delete event form submit.
 *
 * @see mp_booking_busy_event_delete_form()
 */
function mp_booking_busy_event_delete_form_submit(&$form, &$form_state) {
  global $user;
  $values = $form_state['values'];

  $event = bat_Event_load($values['event_id']);
  bat_event_delete($event);

  $form_state['redirect'] = '/user/' . $user->uid;
}
