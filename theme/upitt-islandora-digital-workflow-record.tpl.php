<div class="<?php print $class; ?>">
    <b><?php print $title; ?></b><hr />
<?php foreach ($row as $key => $value): ?>
    <b><?php print $key; ?>:</b> <?php print $value; ?><br />
<?php endforeach; ?>
  <?php if ($extra_content): ?>
    <?php print $extra_content; ?>
    <br class="clearfloat" />
  <?php endif; ?>
</div>
