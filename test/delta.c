#include <glib.h>
#include <locale.h>

typedef struct {
  int a;
} DeltaFixture;

static void
delta_fixture_set_up (DeltaFixture  *fixture,
                      gconstpointer  user_data)
{
    fixture->a = 4;
}

static void
delta_fixture_tear_down (DeltaFixture  *fixture,
                         gconstpointer  user_data)
{
    fixture->a = 0;
}

static void
test_delta_test1 (DeltaFixture  *fixture,
                  gconstpointer  user_data)
{
    g_assert_true (fixture->a == 4);
}

static void
test_delta_test2 (DeltaFixture  *fixture,
                  gconstpointer  user_data)
{
    g_assert_true (fixture->a != 5);
}

int
main (int argc, char *argv[])
{
    setlocale (LC_ALL, "");

    g_test_init (&argc, &argv, NULL);

    // Define the tests.
    g_test_add ("/text-engine/delta/test1", DeltaFixture, "some-user-data",
              delta_fixture_set_up, test_delta_test1,
              delta_fixture_tear_down);
    g_test_add ("/text-engine/delta/test2", DeltaFixture, "some-user-data",
              delta_fixture_set_up, test_delta_test2,
              delta_fixture_tear_down);

    return g_test_run ();
}

