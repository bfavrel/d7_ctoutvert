<?php

class FormWidgetCtoutvertSearch extends FormWidgetPDisposSearch
{
    public function render(array $configuration, array $stored_values, array $default_value) {
        
        $element = parent::render($configuration, $stored_values, $default_value);

        $element['types'] = array(
            '#type' => 'checkboxes',
            '#title' => t("Type"),
            '#options' => array(
                'E' => t("Pitch"),
                'L' => t("Rental"),
            ),
            '#default_value' => array_key_exists('types', $default_value) ? $default_value['types'] : array('E', 'L'),
            '#weight' => 21,
        );
        
        return $element;
    }
}