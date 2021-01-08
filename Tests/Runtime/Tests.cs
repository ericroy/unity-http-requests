using System.Collections;
using System.Collections.Generic;
using NUnit.Framework;
//using UnityEngine;
//using UnityEngine.TestTools;

using UnityHttpRequests;

namespace Tests
{
    public class TestSuite
    {
        private HttpContext context;

        [SetUp]
        public void Setup()
        {
            context = new HttpContext();
        }

        [TearDown]
        public void Teardown()
        {
            if (context != null)
            {
                context.Dispose();
            }
        }

        [Test]
        public void GetRequest()
        {
            var headers = new Dictionary<string, string>();
            headers["User-Agent"] = "FooAgent";
            headers["Accept"] = "application/json";
            var rid = context.Get("https://hookb.in/NOV6KErB8KCWZZpRgE7K", headers);

            bool done = false;
            context.RequestComplete += (ref Response res) =>
            {
                Assert.AreEqual(rid, res.RequestId, "Request id mismatch");
                Assert.AreEqual(200, res.HttpStatus);

                Assert.IsTrue(res.Headers.HeaderEquals("Content-Type", "application/json; charset=utf-8"));

                var index = res.Headers.FindHeader("Content-Type");
                Assert.IsTrue(index >= 0);
                Assert.AreEqual("application/json; charset=utf-8", res.Headers.GetHeaderAlloc(index));

                string headerName;
                string headerValue;
                res.Headers.GetHeaderAlloc(index, out headerName, out headerValue);
                Assert.AreEqual("Content-Type", headerName);
                Assert.AreEqual("application/json; charset=utf-8", headerValue);

                string headerValue2;
                Assert.IsTrue(res.Headers.TryGetHeaderAlloc("Content-Type", out headerValue2));
                Assert.AreEqual("application/json; charset=utf-8", headerValue2);

                Assert.IsTrue(res.Body.ToStringAlloc().Contains("success"));

                done = true;
            };

            for (var i = 0; !done && i < 50; ++i)
            {
                context.Update();
                Thread.Sleep(100);
            }

            Assert.IsTrue(done, "Response never arrived");
        }
    }
}
