<?php

class FormFieldCtoutvertSearch extends FormFieldPDisposSearch
{
    public static function getCompatibleWidgets(array $field_definition) {
        return array('ctoutvert_search_widget');
    }

    public function compileUserInputs(array &$raw_value) {// passage par référence car le parent a besoin de pouvoir altérer la valeur
        $user_input = parent::compileUserInputs($raw_value);

        $user_input['values']['types'] = array_values(array_filter($raw_value['types']));

        return $user_input;
    }
}